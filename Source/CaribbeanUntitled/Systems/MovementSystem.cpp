#include "MovementSystem.h"

#include "ActorRegistry.h"
#include "../Events/MovementPayload.h"

#include "../Components/StatsComponent.h"
#include "GameFramework/SpringArmComponent.h"


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

    // Move direction must be resolved before ApplyLook now, since facing
    // is derived from where the character is actually moving.
    const FVector MoveDirection = ResolveMoveDirection(Character, Control);

    ApplyLook(Character, Control, MoveDirection, DeltaTime);

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

void UMovementSystem::ApplyLook(ACharacter* Character, UControlComponent* Control, const FVector& MoveDirection, float DeltaTime)
{
    // PRIORITY 1: Destination seeking
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

    // PRIORITY 2: Explicit look direction (AI control, etc.)
    FVector LookDirection = Control->GetLookDirection();
    LookDirection.Z = 0.0f;
    if (!LookDirection.IsNearlyZero())
    {
        const float TargetYaw = LookDirection.Rotation().Yaw;
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

    // PRIORITY 3: Face movement direction.
    // Mouse look is now owned entirely by CameraSystem - the character no
    // longer turns to match raw mouse delta directly (that was fighting
    // the camera's own orbit and rotating the mesh far faster than the
    // camera could track). Instead, a player-controlled character turns
    // to face wherever it's actually moving, so the mesh always visibly
    // matches the direction you're running/acting in.
    if (Control->ControlType == EControlType::Player && !MoveDirection.IsNearlyZero())
    {
        const float TargetYaw = MoveDirection.Rotation().Yaw;
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

    // Movement is camera-relative: forward/right are derived from the
    // camera boom's current yaw, not the character's own facing. The
    // character's own rotation is now an OUTPUT of movement (see
    // ApplyLook's Priority 3), not an input to it - basing this on the
    // character's own rotation would create a feedback loop where facing
    // and movement direction chase each other.
    float BasisYaw = Character->GetActorRotation().Yaw;
    if (const USpringArmComponent* CameraBoom = Character->FindComponentByClass<USpringArmComponent>())
    {
        BasisYaw = CameraBoom->GetComponentRotation().Yaw;
    }

    const FRotator YawRotation(0.0f, BasisYaw, 0.0f);
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
    // Check if we're currently in the landing state with an active timer
    bool bIsLanding = LandingTimers.Contains(Actor);
    
    ELocomotion CurrentState =
        DetermineLocomotionState(
            Character,
            Movement,
            Control,
            MoveDirection,
            bWantsToSprint,
            bWantsToCrouch,
            GetCurrentLocomotionState(Actor),
            IsJumpInProgress(Actor)
        );

    // Only publish if the state actually changed
    ELocomotion PreviousState = GetCurrentLocomotionState(Actor);
    if (CurrentState != PreviousState)
    {
        // If we're transitioning INTO the Land state, start the timer
        if (CurrentState == ELocomotion::Land && PreviousState == ELocomotion::Fall)
        {
            // Clear any existing timer
            if (FTimerHandle* ExistingTimer = LandingTimers.Find(Actor))
            {
                if (UWorld* World = GetWorld())
                {
                    World->GetTimerManager().ClearTimer(*ExistingTimer);
                }
                LandingTimers.Remove(Actor);
            }

            // Set new timer
            if (UWorld* World = GetWorld())
            {
                TWeakObjectPtr<AActor> WeakActor(Actor);
                FTimerHandle Handle;
                World->GetTimerManager().SetTimer(
                    Handle,
                    FTimerDelegate::CreateUObject(this, &UMovementSystem::ClearLandingState, WeakActor),
                    LandingDuration,
                    false
                );
                LandingTimers.Add(Actor, Handle);
            }
        }

        // Update the stored state
        PreviousLocomotionStates.Add(Actor, CurrentState);

        // Publish the event
        FMovementPayload Payload;
        Payload.Source = Actor;
        Payload.Type = CurrentState;

        UEventBusSubsystem* EventBus = GetEventBus();
        if (EventBus)
        {
            EventBus->Publish(Payload);
        }
    }
}

void UMovementSystem::ClearLandingState(TWeakObjectPtr<AActor> WeakActor)
{
    AActor* Actor = WeakActor.Get();
    if (Actor)
    {
        LandingTimers.Remove(Actor);
    }
}

void UMovementSystem::ClearAllTimersForActor(AActor* Actor)
{
    if (!Actor) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    FTimerManager& TimerManager = World->GetTimerManager();
    
    // Clear landing timer
    if (FTimerHandle* Handle = LandingTimers.Find(Actor))
    {
        TimerManager.ClearTimer(*Handle);
        LandingTimers.Remove(Actor);
    }
    
    // Clear jump timers (existing code)
    if (FTimerHandle* Handle = PendingJumpTimers.Find(Actor))
    {
        TimerManager.ClearTimer(*Handle);
        PendingJumpTimers.Remove(Actor);
    }
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

    // ✅ FIX: Use actual velocity to determine if moving
    const float Speed = Movement->Velocity.Size();
    const bool bIsMoving = Speed > 10.0f;  // Small threshold for floating point
    
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
