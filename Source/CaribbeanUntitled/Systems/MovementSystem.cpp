#include "MovementSystem.h"

#include "ActorRegistry.h"
#include "../Events/MovementPayload.h"

#include "../Components/StatsComponent.h"


void UMovementSystem::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();
    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors =
        Registry->GetActorsWithComponent<UControlComponent>();

    if (!Actors)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *Actors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            ProcessActor(Actor, DeltaTime);
        }
    }
}


void UMovementSystem::ProcessActor(AActor* Actor, float DeltaTime)
{
    if (!IsValid(Actor))
    {
        return;
    }

    UControlComponent* Control = Actor->FindComponentByClass<UControlComponent>();
    UStatsComponent* Stats = Actor->FindComponentByClass<UStatsComponent>();
    UCharacterMovementComponent* Movement = Actor->FindComponentByClass<UCharacterMovementComponent>();
    ACharacter* Character = Cast<ACharacter>(Actor);

    if (!Control || !Stats || !Movement || !Character)
    {
        return;
    }

    ApplyLook(Character, Control, DeltaTime);

    const FVector MoveDirection = ResolveMoveDirection(Character, Control);
    const bool bWantsToSprint = Control->WantsToSprint();
    const bool bWantsToCrouch = Control->WantsToCrouch();

    ApplySpeed(Stats, Movement, DeltaTime, bWantsToSprint, bWantsToCrouch, !MoveDirection.IsNearlyZero());
    ApplyJump(Character, Stats, Movement, Control);
    ApplyMovement(Character, MoveDirection);
    ApplyCrouch(Character, bWantsToCrouch);

    DetermineAndPublishLocomotion(Actor, Character, Movement, Control, MoveDirection, bWantsToSprint, bWantsToCrouch);
}


//=========================================================================
// Look
//=========================================================================

void UMovementSystem::ApplyLook(ACharacter* Character, UControlComponent* Control, float DeltaTime)
{
    // PRIORITY 1: Mouse/Joystick Input
    const FVector2D LookInput = Control->GetLookInput();
    if (!FMath::IsNearlyZero(LookInput.X) || !FMath::IsNearlyZero(LookInput.Y))
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyLook - Using Mouse Input: X=%f, Y=%f"),
               LookInput.X, LookInput.Y);
        
        FRotator Rotation = Character->GetActorRotation();
        
        // Horizontal rotation (Yaw)
        Rotation.Yaw += LookInput.X;
        
        // Optional: Vertical rotation (Pitch)
        // Rotation.Pitch += LookInput.Y;
        // Rotation.Pitch = FMath::Clamp(Rotation.Pitch, -89.0f, 89.0f);
        
        Character->SetActorRotation(FRotator(0.0f, Rotation.Yaw, 0.0f));
        Control->ClearLookInput();
        return; // Don't process other look sources
    }
    
    // PRIORITY 2: Destination seeking
    if (Control->IsSeekingDestination())
    {
        FVector ToTarget = Control->GetDesiredWorldLocation() - Character->GetActorLocation();
        ToTarget.Z = 0.0f;

        if (!ToTarget.IsNearlyZero())
        {
            FVector FaceDirection = ToTarget.GetSafeNormal();
            const float TargetYaw = FaceDirection.Rotation().Yaw;
            const FRotator CurrentRotation = Character->GetActorRotation();

            const FRotator NewRotation = FMath::RInterpTo(
                CurrentRotation,
                FRotator(0.0f, TargetYaw, 0.0f),
                DeltaTime,
                Control->LookRotationSpeed
            );

            Character->SetActorRotation(NewRotation);
            return;
        }
    }
    
    // PRIORITY 3: Explicit look direction (AI control, etc.)
    FVector LookDirection = Control->GetLookDirection();
    LookDirection.Z = 0.0f;
    if (!LookDirection.IsNearlyZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyLook - Using Explicit Look Direction"));
        const float TargetYaw = LookDirection.Rotation().Yaw;
        const FRotator CurrentRotation = Character->GetActorRotation();

        const FRotator NewRotation = FMath::RInterpTo(
            CurrentRotation,
            FRotator(0.0f, TargetYaw, 0.0f),
            DeltaTime,
            Control->LookRotationSpeed
        );

        Character->SetActorRotation(NewRotation);
    }
}

//=========================================================================
// Movement Direction
//=========================================================================

FVector UMovementSystem::ResolveMoveDirection(ACharacter* Character, UControlComponent* Control) const
{
    const FVector IntentDirection = ComputeIntentDirection(Character, Control->MoveIntent);
    const FVector ExternalDirection = ResolveExternalDirection(Character, Control);

    const bool bHasIntent = !IntentDirection.IsNearlyZero();
    const bool bHasExternal = !ExternalDirection.IsNearlyZero();
    const float Strength = FMath::Clamp(Control->GetDirectionControlStrength(), 0.0f, 1.0f);

    // If no blend needed or only one source exists
    if (!bHasExternal) return IntentDirection;
    if (!bHasIntent) return ExternalDirection;
    if (Strength <= 0.0f) return ExternalDirection;  // Full mouse control
    if (Strength >= 1.0f) return IntentDirection;     // Full keyboard control

    // Blend between both sources
    FVector Result = FMath::Lerp(ExternalDirection, IntentDirection, Strength);
    Result.Normalize();
    return Result;
}

FVector UMovementSystem::ResolveExternalDirection(ACharacter* Character, UControlComponent* Control) const
{
    if (Control->IsSeekingDestination())
    {
        FVector ToTarget = Control->GetDesiredWorldLocation() - Character->GetActorLocation();
        ToTarget.Z = 0.0f;

        if (ToTarget.SizeSquared() <= FMath::Square(Control->GetDirectionAcceptanceRadius()))
        {
            Control->ClearDestination();
            return FVector::ZeroVector;
        }

        ToTarget.Normalize();
        Control->SetWorldMoveDirection(ToTarget);

        return ToTarget;
    }

    FVector Direction = Control->GetWorldMoveDirection();
    Direction.Z = 0.0f;
    return Direction;
}

FVector UMovementSystem::ComputeIntentDirection(ACharacter* Character, const FVector2D& MoveIntent) const
{
    if (MoveIntent.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }

    const FRotator YawRotation(0.0f, Character->GetActorRotation().Yaw, 0.0f);
    const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    FVector Direction = (Forward * MoveIntent.Y) + (Right * MoveIntent.X);
    Direction.Z = 0.0f;

    return Direction;
}


//=========================================================================
// Speed
//=========================================================================

void UMovementSystem::ApplySpeed(
    UStatsComponent* Stats,
    UCharacterMovementComponent* Movement,
    float DeltaTime,
    bool bWantsToSprint,
    bool bWantsToCrouch,
    bool bIsMoving)
{
    float TargetSpeed = Stats->GetWalkSpeed();

    if (bWantsToCrouch)
    {
        TargetSpeed = Stats->GetCrouchSpeed();
    }
    else if (bWantsToSprint)
    {
        TargetSpeed = Stats->GetSprintSpeed();

        if (bIsMoving)
        {
            const float Cost = Stats->GetSprintStaminaCostPerSecond() * DeltaTime;

            if (!Stats->ConsumeStamina(Cost))
            {
                TargetSpeed = Stats->GetWalkSpeed();
            }
        }
    }

    Movement->MaxWalkSpeed = TargetSpeed;
}


//=========================================================================
// Jump
//=========================================================================

void UMovementSystem::ApplyJump(
    ACharacter* Character,
    UStatsComponent* Stats,
    UCharacterMovementComponent* Movement,
    UControlComponent* Control)
{
    Movement->JumpZVelocity = Stats->GetJumpStrength();

    // Restore normal gravity once the ascent is over.
    if (AscendingJumpers.Contains(Character))
    {
        if (Movement->Velocity.Z <= 0.0f || Movement->IsMovingOnGround())
        {
            Movement->GravityScale = 1.0f;
            AscendingJumpers.Remove(Character);
        }
    }

    // Airborne for any reason — cancel any stale queued jump so it can't
    // fire unexpectedly on a later, unrelated landing.
    if (!Movement->IsMovingOnGround())
    {
        if (FTimerHandle* Handle = PendingJumpTimers.Find(Character))
        {
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().ClearTimer(*Handle);
            }
            PendingJumpTimers.Remove(Character);
        }
        return;
    }

    const bool bWantsToJump =
        Control->HasIntent(EControlIntent::Jump) && Movement->IsMovingOnGround();

    if (!bWantsToJump)
    {
        return;
    }

    // Can't start a new jump while already mid-ascent, already queued,
    // or still resolving Fall/Land — must return to a neutral grounded
    // state (Idle/Walk/Sprint/Crouch) before jumping again.
    const ELocomotion CurrentLocomotion = GetCurrentLocomotionState(Character);
    const bool bInFallOrLand =
        CurrentLocomotion == ELocomotion::Fall || CurrentLocomotion == ELocomotion::Land;

    if (AscendingJumpers.Contains(Character) || PendingJumpTimers.Contains(Character) || bInFallOrLand)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        Character->Jump();
        Movement->GravityScale = Stats->JumpAscentGravityScale;
        AscendingJumpers.Add(Character);
        return;
    }

    TWeakObjectPtr<AActor> WeakActor(Character);
    const float AscentGravityScale = Stats->JumpAscentGravityScale;

    FTimerHandle Handle;
    World->GetTimerManager().SetTimer(
        Handle,
        FTimerDelegate::CreateUObject(this, &UMovementSystem::PerformDelayedJump, WeakActor, AscentGravityScale),
        Stats->JumpAnticipationDelay,
        false
    );

    PendingJumpTimers.Add(Character, Handle);
}

void UMovementSystem::PerformDelayedJump(TWeakObjectPtr<AActor> WeakActor, float AscentGravityScale)
{
    AActor* Actor = WeakActor.Get();
    if (Actor)
    {
        PendingJumpTimers.Remove(Actor);
    }

    ACharacter* Character = Cast<ACharacter>(Actor);
    if (!Character)
    {
        return;
    }

    UCharacterMovementComponent* Movement = Character->FindComponentByClass<UCharacterMovementComponent>();
    if (!Movement)
    {
        return;
    }

    // Re-check grounded state — actor may have walked off a ledge, been
    // knocked airborne, etc. during the delay.
    if (!Movement->IsMovingOnGround())
    {
        return;
    }

    Character->Jump();
    Movement->GravityScale = AscentGravityScale;
    AscendingJumpers.Add(Character);
}



//=========================================================================
// Movement Application
//=========================================================================

void UMovementSystem::ApplyMovement(ACharacter* Character, const FVector& MoveDirection)
{
    if (!MoveDirection.IsNearlyZero())
    {
        Character->AddMovementInput(MoveDirection, 1.0f);
    }
}


//=========================================================================
// Crouch
//=========================================================================

void UMovementSystem::ApplyCrouch(ACharacter* Character, bool bWantsToCrouch)
{
    if (bWantsToCrouch && !Character->bIsCrouched)
    {
        Character->Crouch();
    }
    else if (!bWantsToCrouch && Character->bIsCrouched)
    {
        Character->UnCrouch();
    }
}


//=========================================================================
// Locomotion State
//=========================================================================

void UMovementSystem::DetermineAndPublishLocomotion(
    AActor* Actor,
    ACharacter* Character,
    UCharacterMovementComponent* Movement,
    UControlComponent* Control,
    const FVector& MoveDirection,
    bool bWantsToSprint,
    bool bWantsToCrouch)
{
    ELocomotion CurrentState =
        DetermineLocomotionState(
            Character,
            Movement,
            Control,
            MoveDirection,
            bWantsToSprint,
            bWantsToCrouch,
            ELocomotion::Idle,
            IsJumpInProgress(Actor)
        );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("MOVEMENT -> %d"),
        static_cast<uint8>(CurrentState)
    );

    FMovementPayload Payload;
    Payload.Source = Actor;
    Payload.Type = CurrentState;

    GetWorld()
        ->GetSubsystem<UEventBusSubsystem>()
        ->Publish(Payload);
}

ELocomotion UMovementSystem::DetermineLocomotionState(
    ACharacter* Character,
    UCharacterMovementComponent* Movement,
    UControlComponent* Control,
    const FVector& MoveDirection,
    bool bWantsToSprint,
    bool bWantsToCrouch,
    ELocomotion PreviousState,
    bool bJumpInProgress) const
{
    if (Movement->IsFalling())
    {
        return ELocomotion::Fall;
    }

    if (PreviousState == ELocomotion::Fall)
    {
        return ELocomotion::Land;
    }

    // While anticipating (delayed) or ascending a jump, hold the Jump
    // state — the character may still be grounded during the
    // anticipation delay, but we don't want that to fall through to
    // Idle/Walk/etc. and interrupt the jump animation.
    if (bJumpInProgress)
    {
        return ELocomotion::Jump;
    }

    const bool bBlockedFromJumping =
        PreviousState == ELocomotion::Jump ||
        PreviousState == ELocomotion::Fall ||
        PreviousState == ELocomotion::Land;

    if (!bBlockedFromJumping &&
        Control->HasIntent(EControlIntent::Jump) &&
        Movement->IsMovingOnGround())
    {
        return ELocomotion::Jump;
    }

    if (bWantsToCrouch || Character->bIsCrouched)
    {
        return ELocomotion::Crouch;
    }

    const bool bIsMoving = !MoveDirection.IsNearlyZero();
    if (bIsMoving)
    {
        UStatsComponent* Stats = Character->FindComponentByClass<UStatsComponent>();
        if (bWantsToSprint && Movement->MaxWalkSpeed >= Stats->GetSprintSpeed())
        {
            return ELocomotion::Sprint;
        }
        return ELocomotion::Walk;
    }

    return ELocomotion::Idle;
}

UEventBusSubsystem* UMovementSystem::GetEventBus() const
{
    const UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UEventBusSubsystem>() : nullptr;
}

ELocomotion UMovementSystem::GetCurrentLocomotionState(AActor* Actor) const
{
    const ELocomotion* State = PreviousLocomotionStates.Find(Actor);
    return State ? *State : ELocomotion::Idle;
}

bool UMovementSystem::IsJumpInProgress(AActor* Actor) const
{
    return PendingJumpTimers.Contains(Actor) || AscendingJumpers.Contains(Actor);
}
