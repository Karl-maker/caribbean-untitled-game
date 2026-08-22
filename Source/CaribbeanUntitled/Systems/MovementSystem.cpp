#include "MovementSystem.h"

#include "ActorRegistry.h"
#include "../Events/MovementPayload.h"

#include "../Components/StatsComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Pawn.h"


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

    const UWorld* World = Actor->GetWorld();
    const bool bHasAuthority = World && World->GetNetMode() != NM_Client;

    if (Control->ControlType == EControlType::Player)
    {
        const APawn* Pawn = Cast<APawn>(Actor);

        // Player-controlled actors still need server-side processing so
        // the authoritative locomotion state can replicate to the other
        // clients. Only skip them on non-authoritative worlds when this
        // pawn is not locally controlled.
        if (!bHasAuthority && (!Pawn || !Pawn->IsLocallyControlled()))
        {
            return;
        }
    }
    else if (!bHasAuthority)
    {
        return;
    }

    // Move direction must be resolved before movement is applied.
    const FVector MoveDirection = ResolveMoveDirection(Character, Control);

    const bool bWantsToSprint = Control->WantsToSprint();
    const bool bWantsToCrouch = Control->WantsToCrouch();

    ApplySpeed(Stats, Movement, bWantsToSprint, bWantsToCrouch);
    ApplyJump(Character, Stats, Movement, Control);
    ApplyMovement(Character, MoveDirection);
    ApplyCrouch(Character, bWantsToCrouch);

    DetermineAndPublishLocomotion(Actor, Character, Movement, Control, MoveDirection, bWantsToSprint, bWantsToCrouch);
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
    // synced view yaw, not the character's current facing. That lets the
    // server resolve the exact same movement direction as the owning client.
    float BasisYaw = Character->GetActorRotation().Yaw;
    if (const UControlComponent* Control = Character->FindComponentByClass<UControlComponent>())
    {
        BasisYaw = Control->GetViewYaw();
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
    bool bWantsToSprint,
    bool bWantsToCrouch)
{
    float TargetSpeed = Stats->GetWalkSpeed();

    if (bWantsToCrouch)
    {
        TargetSpeed = Stats->GetCrouchSpeed();
    }
    else if (bWantsToSprint)
    {
        TargetSpeed = Stats->GetSprintSpeed();
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

    // Publish the jump state immediately so remote clients receive the
    // start-jump animation before the character transitions into Fall.
    Control->SetReplicatedLocomotion(ELocomotion::Jump);

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
    if (!Character)
    {
        return;
    }

    // During jump anticipation, the character is winding up.
    // Temporarily stop horizontal movement so the wind-up feels deliberate.
    if (PendingJumpTimers.Contains(Character))
    {
        if (UCharacterMovementComponent* Movement =
            Character->FindComponentByClass<UCharacterMovementComponent>())
        {
            FVector Velocity = Movement->Velocity;
            Velocity.X = 0.0f;
            Velocity.Y = 0.0f;
            Movement->Velocity = Velocity;
        }

        return;
    }

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
        Control->SetReplicatedLocomotion(CurrentState);

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
    if (!Character || !Movement || !Control)
    {
        return ELocomotion::Idle;
    }

    // ------------------------------------------------------------
    // LANDING
    // ------------------------------------------------------------
    // LandingTimers is the authoritative hold period for the landing
    // animation. Keep returning Land until the timer expires.
    if (LandingTimers.Contains(Character))
    {
        return ELocomotion::Land;
    }

    // ------------------------------------------------------------
    // AIRBORNE
    // ------------------------------------------------------------
    if (Movement->IsFalling())
    {
        return ELocomotion::Fall;
    }

    // We have just transitioned from airborne -> grounded.
    // Enter the landing state and let DetermineAndPublishLocomotion()
    // create the landing timer.
    if (PreviousState == ELocomotion::Fall)
    {
        return ELocomotion::Land;
    }

    // ------------------------------------------------------------
    // JUMP
    // ------------------------------------------------------------
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

    // ------------------------------------------------------------
    // CROUCH
    // ------------------------------------------------------------
    if (bWantsToCrouch || Character->bIsCrouched)
    {
        return ELocomotion::Crouch;
    }

    // ------------------------------------------------------------
    // LOCOMOTION
    // ------------------------------------------------------------
    const float Speed = Movement->Velocity.Size();
    const bool bIsMoving = Speed > 10.0f;

    if (bIsMoving)
    {
        const float ForwardAmount = MoveDirection | Character->GetActorForwardVector();
        const float RightAmount = MoveDirection | Character->GetActorRightVector();
        const float LateralAbs = FMath::Abs(RightAmount);
        const float ForwardAbs = FMath::Abs(ForwardAmount);

        if (LateralAbs > ForwardAbs + 0.05f)
        {
            return RightAmount >= 0.0f
                ? ELocomotion::StrafeRight
                : ELocomotion::StrafeLeft;
        }

        UStatsComponent* Stats =
            Character->FindComponentByClass<UStatsComponent>();

        if (Stats &&
            bWantsToSprint &&
            Movement->MaxWalkSpeed >= Stats->GetSprintSpeed())
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

void UMovementSystem::Cleanup()
{
    UWorld* World = GetWorld();

    if (World)
    {
        FTimerManager& TimerManager = World->GetTimerManager();

        for (auto& Pair : PendingJumpTimers)
        {
            TimerManager.ClearTimer(Pair.Value);
        }

        for (auto& Pair : LandingTimers)
        {
            TimerManager.ClearTimer(Pair.Value);
        }
    }

    PendingJumpTimers.Empty();
    LandingTimers.Empty();
    AscendingJumpers.Empty();
    PreviousLocomotionStates.Empty();
}
