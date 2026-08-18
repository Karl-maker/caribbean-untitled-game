// CameraSystem.cpp
#include "CameraSystem.h"
#include "../Components/StatsComponent.h"
#include "../Components/ControlComponent.h"
#include "ActorRegistry.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

namespace
{
    // Exponential smoothing toward Target, taking the shortest angular
    // path and always returning a normalized [-180, 180] result. Using a
    // plain FMath::Lerp/FInterpTo on raw (unwrapped, possibly >180 or
    // <-180) yaw values is what causes a camera to spin the "long way"
    // around when it tries to catch up to a normalized target angle.
    float SmoothAngleTo(float Current, float Target, float DeltaTime, float Speed)
    {
        const float Delta = FMath::FindDeltaAngleDegrees(Current, Target);
        const float Alpha = FMath::Clamp(1.0f - FMath::Exp(-Speed * DeltaTime), 0.0f, 1.0f);
        return FRotator::NormalizeAxis(Current + Delta * Alpha);
    }
}

UCameraSystem::UCameraSystem()
{
    // Default OTS camera settings
    NormalOffset = FVector(0.0f, 120.0f, 70.0f);
    NormalArmLength = 220.0f;

    RunningOffset = FVector(0.0f, 25.0f, 55.0f);
    RunningArmLength = 245.0f;

    ActionOffset = FVector(0.0f, 80.0f, 45.0f);
    ActionArmLength = 240.0f;

    MinArmLength = 180.0f;
    MaxArmLength = 290.0f;

    SpeedZoomThreshold = 150.0f;
    MaxSpeedForZoom = 700.0f;

    // Lower = slower / smoother camera movement
    PositionDamping = 5.0f;
    ArmLengthDamping = 4.0f;
    RotationDamping = 7.0f;

    JumpSmoothTime = 0.4f;
    LastJumpTime = -1.0f;
    MouseSensitivity = 0.2f;

    UpdateInterval = 1;
    TickCounter = 0;
}

void UCameraSystem::Tick(UWorld* World, float DeltaTime)
{
    if (!World || DeltaTime <= 0.0f)
    {
        return;
    }

    TickCounter++;
    if (TickCounter % UpdateInterval != 0)
    {
        return;
    }

    UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();
    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors = Registry->GetActorsWithComponent<USpringArmComponent>();
    if (!Actors)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *Actors)
    {
        AActor* Actor = WeakActor.Get();
        if (IsValid(Actor))
        {
            // Only drive the local player's camera. Remote actors should
            // never affect the owning client's camera state.
            if (const APawn* Pawn = Cast<APawn>(Actor))
            {
                if (!Pawn->IsLocallyControlled())
                {
                    continue;
                }
            }
            else if (CameraTarget.IsValid() && CameraTarget.Get() != Actor)
            {
                continue;
            }

            ProcessActor(Actor, DeltaTime);
        }
    }

    CleanupInvalidActors();
}

void UCameraSystem::ProcessActor(AActor* Actor, float DeltaTime)
{
    if (!IsValid(Actor))
    {
        return;
    }

    USpringArmComponent* CameraBoom = Actor->FindComponentByClass<USpringArmComponent>();
    if (!CameraBoom)
    {
        return;
    }

    UpdateCameraState(Actor, CameraBoom, DeltaTime);
}

void UCameraSystem::UpdateCameraState(AActor* Actor, USpringArmComponent* CameraBoom, float DeltaTime)
{
    FCameraState& State = CameraStates.FindOrAdd(Actor);

    // Initialize state if needed
    if (!State.bInitialized)
    {
        State.CurrentArmLength = CameraBoom->TargetArmLength > 0.0f ? CameraBoom->TargetArmLength : NormalArmLength;
        State.CurrentOffset = NormalOffset;

        // Seed orbit angles from the actor's current facing so the camera
        // doesn't snap on the first frame.
        State.CurrentYaw = Actor->GetActorRotation().Yaw;
        State.CurrentPitch = 0.0f;
        State.TargetYaw = State.CurrentYaw;
        State.TargetPitch = 0.0f;
        State.TimeSinceLastLookInput = 0.0f;

        // Decouple the boom from the owning actor's rotation/location
        // entirely. The actor (character) may rotate independently
        // (e.g. via mouse look or movement facing) - we don't want that
        // rotation compounding with the camera's own orbit rotation, or
        // the OTS offset spinning with a body rotation the camera isn't
        // tracking. We compute world location/rotation ourselves every
        // tick instead.
        CameraBoom->SetUsingAbsoluteLocation(true);
        CameraBoom->SetUsingAbsoluteRotation(true);

        // CRITICAL: USpringArmComponent overwrites its own rotation every
        // tick from the owning Pawn's Controller->GetControlRotation()
        // whenever bUsePawnControlRotation is true - regardless of any
        // SetWorldRotation() we call here. Since this project drives look
        // entirely through UControlComponent (never AddControllerYawInput/
        // PitchInput), ControlRotation never changes, so the boom was
        // permanently locked to its default rotation and nothing we did
        // in this system could move it. CameraSystem must be the sole
        // authority over the boom's rotation.
        CameraBoom->bUsePawnControlRotation = false;

        // AAA polish: camera collision so the boom pulls in instead of
        // clipping through walls/geometry.
        CameraBoom->bDoCollisionTest = bCameraCollisionEnabled;
        CameraBoom->ProbeSize = CameraProbeSize;
        CameraBoom->ProbeChannel = ECC_Camera;

        // We drive position/rotation/arm-length smoothing manually below
        // with our own damping curve. Leaving the built-in lag on as well
        // would double-smooth and feel mushy/laggy.
        CameraBoom->bEnableCameraLag = false;
        CameraBoom->bEnableCameraRotationLag = false;

        State.bInitialized = true;
    }

    // Get character reference
    ACharacter* Character = Cast<ACharacter>(Actor);
    UControlComponent* Control = Actor->FindComponentByClass<UControlComponent>();

    // Determine target values (offset and arm length)
    FVector TargetOffset;
    float TargetArmLength;
    DetermineTargetValues(Actor, TargetOffset, TargetArmLength, DeltaTime);

    // Store targets
    State.TargetOffset = TargetOffset;
    State.TargetArmLength = TargetArmLength;

    // Update camera orbit with mouse/control input (only if character exists)
    if (Character)
    {
        UpdateCameraOrbit(Character, Control, State, DeltaTime);
    }

    // Calculate the camera rotation from orbit angles
    State.TargetRotation = FRotator(State.TargetPitch, State.TargetYaw, 0.0f);

    // Smooth interpolation
    const float PosSmoothFactor = FMath::Clamp(1.0f - FMath::Exp(-PositionDamping * DeltaTime), 0.0f, 1.0f);
    const float ArmSmoothFactor = FMath::Clamp(1.0f - FMath::Exp(-ArmLengthDamping * DeltaTime), 0.0f, 1.0f);

    // Smooth position (offset) and arm length - not angular, plain lerp is fine
    State.CurrentOffset = FMath::Lerp(State.CurrentOffset, State.TargetOffset, PosSmoothFactor);
    State.CurrentArmLength = FMath::Lerp(State.CurrentArmLength, State.TargetArmLength, ArmSmoothFactor);

    // Smooth rotation - angle-safe, shortest-path, always normalized.
    // This is the key fix for the camera "spinning the wrong way".
    State.CurrentYaw = SmoothAngleTo(State.CurrentYaw, State.TargetYaw, DeltaTime, RotationDamping);
    State.CurrentPitch = SmoothAngleTo(State.CurrentPitch, State.TargetPitch, DeltaTime, RotationDamping);
    State.CurrentRotation = FRotator(State.CurrentPitch, State.CurrentYaw, 0.0f);

    // ------------------------------------------------------------
    // Apply to camera boom in WORLD space.
    //
    // The OTS offset's horizontal component (X/Y) rotates WITH the
    // camera's own yaw, so the shoulder framing orbits along with you.
    // The vertical component (Z) stays in world space so pitching the
    // camera up/down doesn't skew your eye height.
    // ------------------------------------------------------------
    const FRotator YawOnlyRotation(0.0f, State.CurrentYaw, 0.0f);
    const FVector HorizontalOffset = YawOnlyRotation.RotateVector(
        FVector(State.CurrentOffset.X, State.CurrentOffset.Y, 0.0f));
    const FVector WorldPivot = Actor->GetActorLocation() + HorizontalOffset + FVector(0.0f, 0.0f, State.CurrentOffset.Z);

    CameraBoom->SetWorldLocation(WorldPivot);
    CameraBoom->SetWorldRotation(State.CurrentRotation);
    CameraBoom->TargetArmLength = State.CurrentArmLength;
}

void UCameraSystem::UpdateCameraOrbit(ACharacter* Character, UControlComponent* Control, FCameraState& State, float DeltaTime)
{
    if (!Character)
    {
        return;
    }

    // Prefer an explicit manual override (SetMouseLook); otherwise pull
    // straight from the actor's own control component so the camera
    // works standalone without needing extra input plumbing.
    FVector2D LookDelta = MouseLookInput;
    bool bConsumedFromControl = false;
    if (LookDelta.IsNearlyZero() && Control)
    {
        LookDelta = Control->GetLookInput();
        bConsumedFromControl = true;
    }
    MouseLookInput = FVector2D::ZeroVector;

    // The camera is now the sole consumer of mouse look, so it must clear
    // it after reading. Enhanced Input's Triggered event for mouse deltas
    // simply stops firing when the mouse stops moving (no final zero is
    // sent), so if nobody clears this, the last non-zero delta stays
    // parked here and gets silently re-applied every tick forever.
    if (bConsumedFromControl && Control)
    {
        Control->ClearLookInput();
    }

    const bool bHasLookInput = !LookDelta.IsNearlyZero();

    if (bHasLookInput)
    {
        // Horizontal rotation (Yaw) - orbits around the character.
        // Always normalized so it never accumulates past +-180 and
        // never causes a wraparound spin later.
        State.TargetYaw = FRotator::NormalizeAxis(State.TargetYaw + LookDelta.X * MouseSensitivity);

        // Vertical rotation (Pitch) - orbits up/down, clamped to limits.
        State.TargetPitch = FMath::Clamp(State.TargetPitch + LookDelta.Y * MouseSensitivity, MinPitch, MaxPitch);

        State.TimeSinceLastLookInput = 0.0f;
        if (Control)
        {
            Control->SetViewYaw(State.TargetYaw);
        }
        return;
    }

    State.TimeSinceLastLookInput += DeltaTime;

    if (!bAutoRecenterEnabled)
    {
        return;
    }

    // After a brief pause with no look input, while the character is
    // actually moving, gently settle the camera back in behind them -
    // a common AAA touch (Souls-likes, Zelda, etc). Gated behind a delay
    // so it never fights an active orbit.
    const bool bIsMoving = !Character->GetVelocity().IsNearlyZero();
    if (bIsMoving && State.TimeSinceLastLookInput > AutoRecenterDelay)
    {
        const float CharacterYaw = Character->GetActorRotation().Yaw;
        State.TargetYaw = SmoothAngleTo(State.TargetYaw, CharacterYaw, DeltaTime, RecenterSpeed);
        if (Control)
        {
            Control->SetViewYaw(State.TargetYaw);
        }
    }
}

void UCameraSystem::DetermineTargetValues(AActor* Actor, FVector& OutOffset, float& OutArmLength, float DeltaTime)
{
    ACharacter* Character = Cast<ACharacter>(Actor);
    if (!Character)
    {
        OutOffset = NormalOffset;
        OutArmLength = NormalArmLength;
        return;
    }

    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    UStatsComponent* Stats = Actor->FindComponentByClass<UStatsComponent>();
    UControlComponent* Control = Actor->FindComponentByClass<UControlComponent>();

    if (!Stats || !Control)
    {
        OutOffset = NormalOffset;
        OutArmLength = NormalArmLength;
        return;
    }

    // Check jump state
    const float CurrentSpeed = Movement->Velocity.Size();
    const bool bIsJumping = Movement->IsFalling();

    if (bIsJumping)
    {
        LastJumpTime = 0.0f;
    }
    else if (LastJumpTime >= 0.0f)
    {
        LastJumpTime += DeltaTime;
    }

    const bool bIsRecentlyJumping = bIsJumping || (LastJumpTime >= 0.0f && LastJumpTime < JumpSmoothTime);

    // Determine base camera state
    const bool bRunning = IsRunning(Actor);
    const bool bAction = IsActionState(Actor, bIsRecentlyJumping);

    // Select base values
    if (bRunning)
    {
        OutOffset = RunningOffset;
        OutArmLength = RunningArmLength;
    }
    else if (bAction)
    {
        OutOffset = ActionOffset;
        OutArmLength = ActionArmLength;
    }
    else
    {
        OutOffset = NormalOffset;
        OutArmLength = NormalArmLength;
    }

    // Progressive zoom based on speed
    const float SpeedZoomFactor = FMath::Clamp(
        (CurrentSpeed - SpeedZoomThreshold) / (MaxSpeedForZoom - SpeedZoomThreshold),
        0.0f, 1.0f
    );

    const float ProgressiveArmLength = FMath::Lerp(
        OutArmLength,
        FMath::Clamp(OutArmLength * 1.3f, MinArmLength, MaxArmLength),
        SpeedZoomFactor
    );
    OutArmLength = ProgressiveArmLength;

    // Dynamic offset - center camera when running fast
    OutOffset.Y = FMath::Lerp(OutOffset.Y, 0.0f, SpeedZoomFactor * 0.7f);
    OutOffset.Z = FMath::Lerp(OutOffset.Z, OutOffset.Z + 15.0f, SpeedZoomFactor);
}

bool UCameraSystem::IsRunning(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return false;
    }

    ACharacter* Character = Cast<ACharacter>(Actor);
    if (!Character)
    {
        return false;
    }

    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    UStatsComponent* Stats = Actor->FindComponentByClass<UStatsComponent>();
    UControlComponent* Control = Actor->FindComponentByClass<UControlComponent>();

    if (!Movement || !Stats || !Control)
    {
        return false;
    }

    const float CurrentSpeed = Movement->Velocity.Size();
    const float WalkSpeed = Stats->GetWalkSpeed();
    const float SprintSpeed = Stats->GetSprintSpeed();

    const bool bIsMoving = CurrentSpeed > 10.0f;
    const bool bIsRunningSpeed = CurrentSpeed > WalkSpeed * 1.05f;
    const bool bWantsSprint = Control->SprintIntent > 0.0f;
    const bool bSprintIsFaster = SprintSpeed > WalkSpeed;
    const bool bIsWalkingOrRunning = Movement->MovementMode == MOVE_Walking || Movement->MovementMode == MOVE_NavWalking;
    const bool bIsFalling = Movement->IsFalling();

    return bIsMoving && bIsRunningSpeed && bWantsSprint && bSprintIsFaster && bIsWalkingOrRunning && !bIsFalling;
}

bool UCameraSystem::IsActionState(AActor* Actor, bool bIsRecentlyJumping) const
{
    if (!IsValid(Actor))
    {
        return false;
    }

    ACharacter* Character = Cast<ACharacter>(Actor);
    if (!Character)
    {
        return false;
    }

    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    UControlComponent* Control = Actor->FindComponentByClass<UControlComponent>();

    if (!Movement || !Control)
    {
        return false;
    }

    const bool bIsCrouching = Movement->IsCrouching();
    const bool bHasDashIntent = Control->GetDashIntent() > 0.0f;
    const bool bHasCombatIntent = Control->GetAimIntent() > 0.0f || Control->GetFireIntent() > 0.0f || Control->GetReloadIntent() > 0.0f;

    return bIsCrouching || bHasDashIntent || bHasCombatIntent || bIsRecentlyJumping;
}

void UCameraSystem::SetMouseLook(const FVector2D& LookInput)
{
    MouseLookInput = LookInput;
}

void UCameraSystem::SetCameraTarget(AActor* InTarget)
{
    CameraTarget = InTarget;
}

void UCameraSystem::SetUpdateInterval(int32 InTicks)
{
    UpdateInterval = FMath::Max(1, InTicks);
}

void UCameraSystem::SetNormalCamera(const FVector& Offset, float ArmLength)
{
    NormalOffset = Offset;
    NormalArmLength = FMath::Max(80.0f, ArmLength);
}

void UCameraSystem::SetRunningCamera(const FVector& Offset, float ArmLength)
{
    RunningOffset = Offset;
    RunningArmLength = FMath::Max(80.0f, ArmLength);
}

void UCameraSystem::SetActionCamera(const FVector& Offset, float ArmLength)
{
    ActionOffset = Offset;
    ActionArmLength = FMath::Max(80.0f, ArmLength);
}

void UCameraSystem::SetDamping(float InPositionDamping, float InArmLengthDamping, float InRotationDamping)
{
    PositionDamping = FMath::Max(0.1f, InPositionDamping);
    ArmLengthDamping = FMath::Max(0.1f, InArmLengthDamping);
    RotationDamping = FMath::Max(0.1f, InRotationDamping);
}

void UCameraSystem::SetMouseSensitivity(float InSensitivity)
{
    MouseSensitivity = FMath::Max(0.001f, InSensitivity);
}

void UCameraSystem::SetLookLimits(float InMinPitch, float InMaxPitch)
{
    MinPitch = FMath::Clamp(InMinPitch, -89.0f, 89.0f);
    MaxPitch = FMath::Clamp(InMaxPitch, -89.0f, 89.0f);
}

void UCameraSystem::SetJumpSmoothTime(float InJumpSmoothTime)
{
    JumpSmoothTime = FMath::Max(0.0f, InJumpSmoothTime);
}

void UCameraSystem::SetAutoRecenter(bool bEnabled, float Delay, float Speed)
{
    bAutoRecenterEnabled = bEnabled;
    AutoRecenterDelay = FMath::Max(0.0f, Delay);
    RecenterSpeed = FMath::Max(0.1f, Speed);
}

void UCameraSystem::SetCameraCollision(bool bEnabled, float ProbeSize)
{
    bCameraCollisionEnabled = bEnabled;
    CameraProbeSize = FMath::Max(1.0f, ProbeSize);

    // Re-apply to any already-initialized states next tick by resetting
    // the flag isn't necessary here since bDoCollisionTest/ProbeSize are
    // read from the boom itself on next access; but for boom instances
    // already initialized this call updates future frames too since we
    // only set these once at init. If you need it to apply live to
    // already-running actors, call this before their first Tick.
}

void UCameraSystem::CleanupInvalidActors()
{
    for (auto It = CameraStates.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid())
        {
            It.RemoveCurrent();
        }
    }
}
