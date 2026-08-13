#include "MovementSystem.h"

#include "../GameActors/GameActor.h"
#include "ActorRegistry.h"

#include "../Components/ControlComponent.h"
#include "../Components/GameMovementComponent.h"
#include "../Components/MotionComponent.h"

#include "Engine/World.h"


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
     * Start from the ControlComponent index.
     *
     * Only Actors capable of receiving movement/control
     * input are considered.
     */
    const TSet<TWeakObjectPtr<AGameActor>>* ControlActors =
        Registry->GetActorsWithComponent<UControlComponent>();

    if (!ControlActors)
    {
        return;
    }

    for (const TWeakObjectPtr<AGameActor>& WeakActor : *ControlActors)
    {
        AGameActor* Actor = WeakActor.Get();

        if (!IsValid(Actor))
        {
            continue;
        }

        /*
         * Movement requires:
         *
         * Control
         * Movement configuration
         * Motion
         */
        if (!Actor->HasComponent<UGameMovementComponent>() ||
            !Actor->HasComponent<UMotionComponent>())
        {
            continue;
        }

        ProcessActor(
            Actor,
            DeltaTime
        );
    }
}


void UMovementSystem::ProcessActor(
    AGameActor* Actor,
    float DeltaTime)
{
    if (!IsValid(Actor))
    {
        return;
    }

    UControlComponent* Control =
        Actor->GetComponent<UControlComponent>();

    UGameMovementComponent* Movement =
        Actor->GetComponent<UGameMovementComponent>();

    UMotionComponent* Motion =
        Actor->GetComponent<UMotionComponent>();

    if (!Control || !Movement || !Motion)
    {
        return;
    }

    // ============================================================
    // INPUT → DIRECTION
    // ============================================================

    const FVector DesiredDirection =
        CalculateDesiredDirection(
            Actor,
            Control
        );

    Movement->SetDesiredDirection(
        DesiredDirection
    );


    // ============================================================
    // MOVEMENT STATE
    // ============================================================

    ProcessSprint(
        Movement,
        Control
    );

    ProcessCrouch(
        Movement,
        Control
    );


    // ============================================================
    // SPEED
    // ============================================================

    const float TargetSpeed =
        CalculateTargetSpeed(
            Movement,
            Control
        );


    // ============================================================
    // MOVEMENT → MOTION REQUEST
    // ============================================================

    /*
     * MovementSystem does NOT calculate actual velocity.
     *
     * It only tells the physics system what movement is wanted.
     */
    Motion->DesiredVelocity =
        DesiredDirection * TargetSpeed;


    Motion->DesiredDirection =
        DesiredDirection;


    Motion->DesiredSpeed =
        TargetSpeed;


    // ============================================================
    // JUMP REQUEST
    // ============================================================

    ProcessJump(
        Movement,
        Motion,
        Control
    );


    // ============================================================
    // DERIVED REQUEST STATE
    // ============================================================

    Motion->UpdateDerivedState();
}


FVector UMovementSystem::CalculateDesiredDirection(
    AGameActor* Actor,
    const UControlComponent* Control) const
{
    if (!Actor || !Control)
    {
        return FVector::ZeroVector;
    }

    /*
     * AI / Network / Script may provide a world-space
     * movement direction directly.
     */
    if (!Control->MoveDirection.IsNearlyZero())
    {
        FVector Direction =
            Control->MoveDirection;

        Direction.Z = 0.0f;

        return Direction.GetSafeNormal();
    }


    /*
     * Convert local control input into world-space.
     *
     * X = Left / Right
     * Y = Back / Forward
     */
    FVector2D Input =
        Control->MoveIntent;

    Input =
        Input.GetClampedToMaxSize(1.0f);

    if (Input.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }

    const FRotator ActorRotation =
        Actor->GetActorRotation();

    FVector Forward =
        ActorRotation.RotateVector(
            FVector::ForwardVector
        );

    FVector Right =
        ActorRotation.RotateVector(
            FVector::RightVector
        );

    Forward.Z = 0.0f;
    Right.Z = 0.0f;

    Forward.Normalize();
    Right.Normalize();

    FVector Direction =
        Forward * Input.Y +
        Right * Input.X;

    Direction.Z = 0.0f;

    return Direction.GetSafeNormal();
}


float UMovementSystem::CalculateTargetSpeed(
    const UGameMovementComponent* Movement,
    const UControlComponent* Control) const
{
    if (!Movement || !Control)
    {
        return 0.0f;
    }

    if (Control->WantsToCrouch())
    {
        return Movement->CrouchSpeed;
    }

    if (Control->WantsToSprint())
    {
        return Movement->SprintSpeed;
    }

    return Movement->WalkSpeed;
}


void UMovementSystem::ProcessJump(
    UGameMovementComponent* Movement,
    UMotionComponent* Motion,
    const UControlComponent* Control)
{
    if (!Movement || !Motion || !Control)
    {
        return;
    }

    if (!Movement->bCanJump)
    {
        return;
    }

    if (!Movement->IsGrounded())
    {
        return;
    }

    if (Control->JumpIntent <= 0.0f)
    {
        return;
    }

    /*
     * Do NOT modify velocity here.
     *
     * We simply create a jump request.
     *
     * PhysicsSystem will turn this into an actual
     * physical impulse/velocity change.
     */
    Motion->bWantsToJump = true;
}


void UMovementSystem::ProcessSprint(
    UGameMovementComponent* Movement,
    const UControlComponent* Control)
{
    if (!Movement || !Control)
    {
        return;
    }

    const bool bSprint =
        Control->WantsToSprint() &&
        !Control->MoveIntent.IsNearlyZero();

    Movement->SetSprinting(
        bSprint
    );
}


void UMovementSystem::ProcessCrouch(
    UGameMovementComponent* Movement,
    const UControlComponent* Control)
{
    if (!Movement || !Control)
    {
        return;
    }

    Movement->SetCrouching(
        Control->WantsToCrouch()
    );
}
