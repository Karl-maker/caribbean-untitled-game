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

    UActorRegistry* Registry =
        World->GetSubsystem<UActorRegistry>();

    if (!Registry)
    {
        return;
    }

    /*
     * We don't care what kind of Actor this is.
     *
     * If it has a ControlComponent, the movement system
     * considers it.
     */
    const TSet<TWeakObjectPtr<AActor>>* Actors =
        Registry->GetActorsWithComponent<UControlComponent>();

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

        ProcessActor(Actor, DeltaTime);
    }
}


void UMovementSystem::ProcessActor(
    AActor* Actor,
    float DeltaTime)
{
    if (!IsValid(Actor))
    {
        return;
    }


    // ============================================================
    // REQUIRED COMPONENTS
    // ============================================================

    UControlComponent* Control =
        Actor->FindComponentByClass<UControlComponent>();

    UStatsComponent* Stats =
        Actor->FindComponentByClass<UStatsComponent>();

    UCharacterMovementComponent* Movement =
        Actor->FindComponentByClass<UCharacterMovementComponent>();


    if (!Control || !Stats || !Movement)
    {
        return;
    }


    // ============================================================
    // CHARACTER CAPABILITY
    // ============================================================

    /*
     * UCharacterMovementComponent is designed to work with
     * ACharacter.
     *
     * The registry/system still doesn't care what the Actor is.
     * We simply check whether this particular Actor supports
     * Character movement.
     */
    ACharacter* Character =
        Cast<ACharacter>(Actor);

    if (!Character)
    {
        return;
    }


    // ============================================================
    // MOVEMENT INTENT
    // ============================================================

    FVector MoveDirection =
        Control->GetWorldMoveDirection();

    MoveDirection.Z = 0.0f;

    if (!MoveDirection.IsNearlyZero())
    {
        MoveDirection.Normalize();
    }


    // ============================================================
    // DETERMINE MOVEMENT SPEED
    // ============================================================

    /*
     * Movement speeds now belong to CharacterMovementComponent.
     *
     * StatsComponent no longer contains:
     *
     *     WalkSpeed
     *     RunSpeed
     *     SprintSpeed
     *     CrouchSpeed
     *
     * We therefore modify MaxWalkSpeed directly here.
     */

    const float NormalSpeed = 300.0f;
    const float SprintSpeed = 700.0f;
    const float CrouchSpeed = 150.0f;

    float TargetSpeed = NormalSpeed;


    if (Control->WantsToCrouch())
    {
        TargetSpeed = CrouchSpeed;
    }
    else if (Control->WantsToSprint())
    {
        TargetSpeed = SprintSpeed;
    }


    // ============================================================
    // STAMINA
    // ============================================================

    if (Control->WantsToSprint())
    {
        /*
         * Only consume stamina while actually moving.
         */
        if (!MoveDirection.IsNearlyZero())
        {
            const float SprintCostPerSecond = 10.0f;

            const float Cost =
                SprintCostPerSecond * DeltaTime;

            if (!Stats->ConsumeStamina(Cost))
            {
                /*
                 * No stamina.
                 *
                 * Fall back to normal movement speed.
                 */
                TargetSpeed = NormalSpeed;
            }
        }
    }


    // ============================================================
    // APPLY SPEED
    // ============================================================

    Movement->MaxWalkSpeed =
        TargetSpeed;


    // ============================================================
    // APPLY MOVEMENT
    // ============================================================

    if (!MoveDirection.IsNearlyZero())
    {
        Character->AddMovementInput(
            MoveDirection,
            1.0f
        );
    }


    // ============================================================
    // JUMP
    // ============================================================

    if (Control->HasIntent(EControlIntent::Jump))
    {
        if (Movement->IsMovingOnGround())
        {
            Character->Jump();
        }
    }


    // ============================================================
    // CROUCH
    // ============================================================

    if (Control->WantsToCrouch())
    {
        if (!Character->bIsCrouched)
        {
            Character->Crouch();
        }
    }
    else
    {
        if (Character->bIsCrouched)
        {
            Character->UnCrouch();
        }
    }
}
