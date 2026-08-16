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


//=============================================================================
// CONSTRUCTOR
//=============================================================================

UCameraSystem::UCameraSystem()
{
    // Default values - Closer camera, right shoulder (GOW style)
    NormalOffset = FVector(0.0f, 120.0f, 70.0f);   // Closer right offset
    NormalArmLength = 220.0f;                      // Closer
    
    RunningOffset = FVector(0.0f, 0.0f, 50.0f);    // Centered when running
    RunningArmLength = 280.0f;                     // Closer
    
    ActionOffset = FVector(0.0f, 80.0f, 45.0f);    // Closer right for combat
    ActionArmLength = 250.0f;                      // Closer
    
    // Progressive zoom settings - closer range
    MinArmLength = 180.0f;
    MaxArmLength = 350.0f;
    SpeedZoomThreshold = 100.0f;
    MaxSpeedForZoom = 800.0f;
    
    // Smooth damping - slightly slower for less jitter
    PositionDamping = 10.0f;
    ArmLengthDamping = 8.0f;
    OffsetDamping = 6.0f;
    
    // Jump smoothing
    JumpSmoothTime = 0.3f;
    LastJumpTime = -1.0f;
    
    UpdateInterval = 1;
    TickCounter = 0;
}


//=============================================================================
// TICK
//=============================================================================

void UCameraSystem::Tick(
    UWorld* World,
    float DeltaTime
)
{
    if (!World || DeltaTime <= 0.0f)
    {
        return;
    }

    UActorRegistry* Registry =
        World->GetSubsystem<UActorRegistry>();

    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors =
        Registry->GetActorsWithComponent<USpringArmComponent>();

    if (!Actors)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *Actors)
    {
        AActor* Actor = WeakActor.Get();

        if (!IsValid(Actor))
        {
            continue;
        }

        ProcessActor(
            Actor,
            DeltaTime
        );
    }

    CleanupInvalidActors();
}


//=============================================================================
// UPDATE INTERVAL
//=============================================================================

void UCameraSystem::SetUpdateInterval(int32 InTicks)
{
    UpdateInterval =
        FMath::Max(1, InTicks);
}


//=============================================================================
// PROCESS ACTOR
//=============================================================================

void UCameraSystem::ProcessActor(
    AActor* Actor,
    float DeltaTime
)
{
    if (!IsValid(Actor))
    {
        return;
    }

    USpringArmComponent* CameraBoom =
        Actor->FindComponentByClass<USpringArmComponent>();

    if (!CameraBoom)
    {
        return;
    }

    UStatsComponent* Stats =
        Actor->FindComponentByClass<UStatsComponent>();

    if (!Stats)
    {
        return;
    }

    FCameraState& State =
        CameraStates.FindOrAdd(Actor);

    ACharacter* Character = Cast<ACharacter>(Actor);
    float CurrentSpeed = 0.0f;
    bool bIsJumping = false;
    
    if (Character)
    {
        UCharacterMovementComponent* Movement =
            Character->GetCharacterMovement();
        if (Movement)
        {
            CurrentSpeed = Movement->Velocity.Size();
            bIsJumping = Movement->IsFalling();
        }
    }


    // ============================================================
    // JUMP SMOOTHING - Detect jump with hysteresis
    // ============================================================

    // Update jump state with smooth timer
    if (bIsJumping)
    {
        LastJumpTime = 0.0f;  // Reset timer when jumping
    }
    else if (LastJumpTime >= 0.0f)
    {
        LastJumpTime += DeltaTime;  // Increment timer when not jumping
    }
    
    // Only consider jump active if recently jumped (within smooth time)
    const bool bIsRecentlyJumping =
        bIsJumping ||
        (LastJumpTime >= 0.0f && LastJumpTime < JumpSmoothTime);


    // ============================================================
    // DETERMINE BASE CAMERA STATE
    // ============================================================

    const bool bRunning = IsRunning(Actor);
    const bool bAction = IsActionState(Actor, bIsRecentlyJumping);


    // ============================================================
    // SELECT BASE TARGET
    // ============================================================

    FVector TargetOffset;
    float TargetArmLength;

    if (bRunning)
    {
        TargetOffset = RunningOffset;
        TargetArmLength = RunningArmLength;
    }
    else if (bAction)
    {
        TargetOffset = ActionOffset;
        TargetArmLength = ActionArmLength;
    }
    else
    {
        TargetOffset = NormalOffset;
        TargetArmLength = NormalArmLength;
    }


    // ============================================================
    // PROGRESSIVE ZOOM BASED ON SPEED
    // ============================================================

    // Zoom out more as speed increases (GTA style)
    float SpeedZoomFactor = FMath::Clamp(
        (CurrentSpeed - SpeedZoomThreshold) / (MaxSpeedForZoom - SpeedZoomThreshold),
        0.0f, 1.0f
    );
    
    // Apply progressive zoom while preserving the base arm length
    float ProgressiveArmLength = FMath::Lerp(
        TargetArmLength,
        FMath::Clamp(TargetArmLength * 1.3f, MinArmLength, MaxArmLength),  // Less aggressive zoom
        SpeedZoomFactor
    );

    // Calculate dynamic offset based on speed (center the character when running fast)
    FVector DynamicOffset = TargetOffset;
    DynamicOffset.Y = FMath::Lerp(
        TargetOffset.Y,           // Right offset (positive)
        0.0f,                     // Center
        SpeedZoomFactor * 0.7f    // Gradually center as speed increases
    );
    
    // Slightly raise the camera at high speed for better visibility
    DynamicOffset.Z = FMath::Lerp(
        TargetOffset.Z,
        TargetOffset.Z + 15.0f,
        SpeedZoomFactor
    );


    // ============================================================
    // INITIALIZE STATE
    // ============================================================

    if (State.CurrentArmLength <= 0.0f)
    {
        State.CurrentArmLength = CameraBoom->TargetArmLength;
    }

    if (State.CurrentOffset.IsNearlyZero())
    {
        State.CurrentOffset = CameraBoom->GetRelativeLocation();
    }


    // ============================================================
    // POSITION DAMPING
    // ============================================================

    const float PositionSmoothFactor =
        FMath::Clamp(1.0f - FMath::Exp(-PositionDamping * DeltaTime), 0.0f, 1.0f);
    
    State.CurrentOffset =
        FMath::Lerp(
            State.CurrentOffset,
            DynamicOffset,
            PositionSmoothFactor
        );


    // ============================================================
    // ARM LENGTH DAMPING
    // ============================================================

    const float ArmLengthSmoothFactor =
        FMath::Clamp(1.0f - FMath::Exp(-ArmLengthDamping * DeltaTime), 0.0f, 1.0f);
    
    State.CurrentArmLength =
        FMath::Lerp(
            State.CurrentArmLength,
            ProgressiveArmLength,
            ArmLengthSmoothFactor
        );


    // ============================================================
    // APPLY
    // ============================================================

    CameraBoom->SetRelativeLocation(State.CurrentOffset);
    CameraBoom->TargetArmLength = State.CurrentArmLength;
}


//=============================================================================
// RUNNING - Using CharacterMovementComponent actual members
//=============================================================================

bool UCameraSystem::IsRunning(
    AActor* Actor
) const
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

    UCharacterMovementComponent* Movement =
        Character->GetCharacterMovement();
    
    if (!Movement)
    {
        return false;
    }

    UStatsComponent* Stats =
        Actor->FindComponentByClass<UStatsComponent>();

    UControlComponent* Control =
        Actor->FindComponentByClass<UControlComponent>();

    if (!Stats || !Control)
    {
        return false;
    }


    // ============================================================
    // CHECK ACTUAL MOVEMENT SPEED
    // ============================================================

    const float CurrentSpeed = Movement->Velocity.Size();
    const float WalkSpeed = Stats->GetWalkSpeed();
    const float SprintSpeed = Stats->GetSprintSpeed();


    // ============================================================
    // MOVEMENT VALIDATION - With speed threshold hysteresis
    // ============================================================

    // Is the character actually moving (with threshold)
    const bool bIsMoving = CurrentSpeed > 10.0f;

    // Is the character moving faster than walk speed (with hysteresis)
    const bool bIsRunningSpeed = CurrentSpeed > WalkSpeed * 1.05f;  // Lower threshold

    // Is the character using sprint input?
    const bool bWantsSprint = Control->SprintIntent > 0.0f;

    // Is sprint actually faster than walk?
    const bool bSprintIsFaster = SprintSpeed > WalkSpeed;

    // Check if we're in a movement mode that supports sprinting
    const bool bIsWalkingOrRunning =
        Movement->MovementMode == MOVE_Walking ||
        Movement->MovementMode == MOVE_NavWalking;

    // Don't trigger running during jump or landing (with hysteresis)
    const bool bIsFalling = Movement->IsFalling();


    return
        bIsMoving &&
        bIsRunningSpeed &&
        bWantsSprint &&
        bSprintIsFaster &&
        bIsWalkingOrRunning &&
        !bIsFalling;
}


//=============================================================================
// ACTION STATE - With jump dampening
//=============================================================================

bool UCameraSystem::IsActionState(
    AActor* Actor,
    bool bIsRecentlyJumping
) const
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

    UCharacterMovementComponent* Movement =
        Character->GetCharacterMovement();
    
    if (!Movement)
    {
        return false;
    }

    UControlComponent* Control =
        Actor->FindComponentByClass<UControlComponent>();

    if (!Control)
    {
        return false;
    }


    // ============================================================
    // CHECK MOVEMENT STATE - With smooth jump detection
    // ============================================================

    const bool bIsCrouching = Movement->IsCrouching();


    // ============================================================
    // CHECK ACTION INTENTS
    // ============================================================

    const bool bHasDashIntent = Control->GetDashIntent() > 0.0f;

    const bool bHasCombatIntent =
        Control->GetAimIntent() > 0.0f ||
        Control->GetFireIntent() > 0.0f ||
        Control->GetReloadIntent() > 0.0f;


    // ============================================================
    // ACTION STATE - With jump dampening
    // ============================================================

    // Only trigger action state for jump if recently jumping (smooth transition)
    return
        bIsCrouching ||
        bHasDashIntent ||
        bHasCombatIntent ||
        bIsRecentlyJumping;  // Smooth jump detection
}


//=============================================================================
// CLEANUP
//=============================================================================

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


//=============================================================================
// SET NORMAL CAMERA
//=============================================================================

void UCameraSystem::SetNormalCamera(
    const FVector& Offset,
    float ArmLength
)
{
    NormalOffset = Offset;
    NormalArmLength = FMath::Max(80.0f, ArmLength);  // Allow closer
}


//=============================================================================
// SET RUNNING CAMERA
//=============================================================================

void UCameraSystem::SetRunningCamera(
    const FVector& Offset,
    float ArmLength
)
{
    RunningOffset = Offset;
    RunningArmLength = FMath::Max(80.0f, ArmLength);  // Allow closer
}


//=============================================================================
// SET ACTION CAMERA
//=============================================================================

void UCameraSystem::SetActionCamera(
    const FVector& Offset,
    float ArmLength
)
{
    ActionOffset = Offset;
    ActionArmLength = FMath::Max(80.0f, ArmLength);  // Allow closer
}


//=============================================================================
// SET DAMPING
//=============================================================================

void UCameraSystem::SetDamping(
    float InPositionDamping,
    float InArmLengthDamping
)
{
    PositionDamping = FMath::Max(0.1f, InPositionDamping);
    ArmLengthDamping = FMath::Max(0.1f, InArmLengthDamping);
}

//=============================================================================
// SET JUMP SMOOTH TIME
//=============================================================================

void UCameraSystem::SetJumpSmoothTime(float InJumpSmoothTime)
{
    JumpSmoothTime = FMath::Max(0.0f, InJumpSmoothTime);
}
