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
    FVector FaceDirection = FVector::ZeroVector;
    bool bHasExplicitFaceDirection = false;

    if (Control->IsSeekingDestination())
    {
        FVector ToTarget = Control->GetDesiredWorldLocation() - Character->GetActorLocation();
        ToTarget.Z = 0.0f;

        if (!ToTarget.IsNearlyZero())
        {
            FaceDirection = ToTarget.GetSafeNormal();
            bHasExplicitFaceDirection = true;
        }
    }
    else
    {
        FaceDirection = Control->GetLookDirection();
        FaceDirection.Z = 0.0f;
        bHasExplicitFaceDirection = !FaceDirection.IsNearlyZero();
    }

    if (bHasExplicitFaceDirection)
    {
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

    const FVector2D LookInput = Control->GetLookInput();
    if (!FMath::IsNearlyZero(LookInput.X))
    {
        FRotator Rotation = Character->GetActorRotation();
        Rotation.Yaw += LookInput.X;
        Character->SetActorRotation(FRotator(0.0f, Rotation.Yaw, 0.0f));
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

    FVector Result = FVector::ZeroVector;

    if (bHasExternal && bHasIntent)
    {
        const float Strength = FMath::Clamp(Control->GetDirectionControlStrength(), 0.0f, 1.0f);
        Result = FMath::Lerp(ExternalDirection, IntentDirection, Strength);
    }
    else if (bHasExternal)
    {
        Result = ExternalDirection;
    }
    else if (bHasIntent)
    {
        Result = IntentDirection;
    }

    if (!Result.IsNearlyZero())
    {
        Result.Normalize();
    }

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
    ELocomotion* PreviousState = PreviousLocomotionStates.Find(Actor);
    const ELocomotion PrevValue = PreviousState ? *PreviousState : ELocomotion::Idle;
    const bool bJumpInProgress = IsJumpInProgress(Actor);

    ELocomotion CurrentState = DetermineLocomotionState(
        Character, Movement, Control, MoveDirection, bWantsToSprint, bWantsToCrouch, PrevValue, bJumpInProgress);

    if (PreviousState && *PreviousState == CurrentState)
    {
        return;
    }

    PreviousLocomotionStates.Add(Actor, CurrentState);

    UEventBusSubsystem* EventBus = GetEventBus();
    if (EventBus)
    {
        FMovementPayload Payload;
        Payload.Source = Actor;
        Payload.Type = CurrentState;
        EventBus->Publish(Payload);
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
