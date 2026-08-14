#include "MovementSystem.h"

#include "ActorRegistry.h"

#include "../Components/ControlComponent.h"
#include "../Components/StatsComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


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
}


//=========================================================================
// Look
//=========================================================================

void UMovementSystem::ApplyLook(ACharacter* Character, UControlComponent* Control, float DeltaTime)
{
    // If the Actor is currently seeking a destination (MoveToWorldLocation),
    // keep it facing that destination as it moves — recompute every tick
    // from the Actor's live position via SetLookAtLocation, rather than
    // relying on a one-shot direction that goes stale as the Actor moves.
    if (Control->IsSeekingDestination())
    {
        Control->SetLookAtLocation(Control->GetDesiredWorldLocation());
    }

    // Priority 1: an explicit world-space facing direction — either the
    // one just computed above, or one set directly via SetLookDirection()
    // / SetLookAtLocation() from elsewhere (mouse-derived, AI one-shot,
    // etc). Rotate smoothly toward it rather than snapping.
    FVector FaceDirection = Control->GetLookDirection();
    FaceDirection.Z = 0.0f;

    if (!FaceDirection.IsNearlyZero())
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

    // Priority 2: raw per-frame turn delta (e.g. gamepad right stick, or
    // mouse fed as a delta instead of a direction). Only used when nothing
    // above is driving rotation, so inputs don't fight each other.
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
        Result = FMath::Lerp(IntentDirection, ExternalDirection, Strength);
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

    if (Control->HasIntent(EControlIntent::Jump) && Movement->IsMovingOnGround())
    {
        Character->Jump();
    }
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
