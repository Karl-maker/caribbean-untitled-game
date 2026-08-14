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

    ACharacter* Character =
        Cast<ACharacter>(Actor);

    if (!Character)
    {
        return;
    }


    // ============================================================
    // MOUSE LOOK / CHARACTER ROTATION
    // ============================================================
    //
    // LookInput is raw mouse/controller input.
    //
    // X = horizontal mouse movement -> character yaw
    // Y = vertical mouse movement   -> ignored here
    //
    // Movement remains separate from the mouse input.
    // The mouse changes the character's facing direction,
    // and movement is calculated from that facing direction.
    //
    // ============================================================

    const FVector2D LookInput =
        Control->GetLookInput();

    if (!FMath::IsNearlyZero(LookInput.X))
    {
        FRotator CharacterRotation =
            Character->GetActorRotation();

        CharacterRotation.Yaw += LookInput.X;

        Character->SetActorRotation(
            FRotator(
                0.0f,
                CharacterRotation.Yaw,
                0.0f
            )
        );
    }


    // ============================================================
    // MOVEMENT INTENT
    // ============================================================
    //
    // ControlComponent stores movement in LOCAL SPACE:
    //
    // X = Left / Right
    // Y = Back / Forward
    //
    // Movement is calculated using the character's CURRENT
    // facing direction.
    //
    // Therefore:
    //
    // Mouse right -> character turns right
    // W            -> character moves in the new direction
    //
    // ============================================================

    const FVector2D MoveIntent =
        Control->MoveIntent;

    FVector MoveDirection =
        FVector::ZeroVector;

    if (!MoveIntent.IsNearlyZero())
    {
        const FRotator CharacterRotation =
            Character->GetActorRotation();

        // Only use yaw for ground movement.
        const FRotator YawRotation(
            0.0f,
            CharacterRotation.Yaw,
            0.0f
        );

        const FVector Forward =
            FRotationMatrix(YawRotation)
                .GetUnitAxis(EAxis::X);

        const FVector Right =
            FRotationMatrix(YawRotation)
                .GetUnitAxis(EAxis::Y);

        MoveDirection =
            (Forward * MoveIntent.Y) +
            (Right * MoveIntent.X);

        MoveDirection.Z = 0.0f;

        if (!MoveDirection.IsNearlyZero())
        {
            MoveDirection.Normalize();
        }
    }


    // ============================================================
    // MOVEMENT SPEED
    // ============================================================

    /*
     * StatsComponent owns the gameplay speeds.
     *
     * CharacterMovementComponent owns the actual movement
     * simulation and receives the current target speed.
     */

    const float NormalSpeed =
        Stats->GetWalkSpeed();

    const float SprintSpeed =
        Stats->GetSprintSpeed();

    const float CrouchSpeed =
        Stats->GetCrouchSpeed();

    float TargetSpeed =
        NormalSpeed;


    if (Control->WantsToCrouch())
    {
        TargetSpeed =
            CrouchSpeed;
    }
    else if (Control->WantsToSprint())
    {
        TargetSpeed =
            SprintSpeed;
    }


    // ============================================================
    // STAMINA
    // ============================================================

    if (Control->WantsToSprint())
    {
        /*
         * Sprint stamina is only consumed while the character
         * actually has movement input.
         */

        if (!MoveDirection.IsNearlyZero())
        {
            const float SprintCostPerSecond =
                Stats->GetSprintStaminaCostPerSecond();

            const float Cost =
                SprintCostPerSecond * DeltaTime;

            if (!Stats->ConsumeStamina(Cost))
            {
                /*
                 * Insufficient stamina.
                 *
                 * Fall back to normal walking speed.
                 */

                TargetSpeed =
                    NormalSpeed;
            }
        }
    }


    // ============================================================
    // APPLY SPEED
    // ============================================================

    Movement->MaxWalkSpeed =
        TargetSpeed;


    // ============================================================
    // JUMP
    // ============================================================

    Movement->JumpZVelocity =
        Stats->GetJumpStrength();


    if (Control->HasIntent(EControlIntent::Jump))
    {
        if (Movement->IsMovingOnGround())
        {
            Character->Jump();
        }
    }


    // ============================================================
    // MOVEMENT INPUT
    // ============================================================

    if (!MoveDirection.IsNearlyZero())
    {
        Character->AddMovementInput(
            MoveDirection,
            1.0f
        );
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
