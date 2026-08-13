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
     * This means we only inspect Actors that can actually
     * receive movement/control input.
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
         * Movement configuration/state
         * Motion
         */
        if (!Actor->HasComponent<UGameMovementComponent>() ||
            !Actor->HasComponent<UMotionComponent>())
        {
            continue;
        }

        ProcessActor(Actor, DeltaTime);
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
    // CONTROL → MOTION
    // ============================================================

    /*
     * UGameMovementComponent does not store velocity.
     *
     * The movement system writes the requested velocity directly
     * into UMotionComponent.
     */
    Motion->DesiredVelocity =
        DesiredDirection * TargetSpeed;


    // ============================================================
    // ACCELERATION
    // ============================================================

    UpdateHorizontalMotion(
        Movement,
        Motion,
        DesiredDirection,
        TargetSpeed,
        DeltaTime
    );


    // ============================================================
    // JUMP
    // ============================================================

    ProcessJump(
        Movement,
        Motion,
        Control
    );


    // ============================================================
    // DERIVED MOTION STATE
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
     * AI / Network / Script may provide a world-space direction
     * directly.
     */
    if (!Control->MoveDirection.IsNearlyZero())
    {
        FVector Direction =
            Control->MoveDirection;

        Direction.Z = 0.0f;

        return Direction.GetSafeNormal();
    }


    /*
     * Otherwise convert local control input into world-space.
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


void UMovementSystem::UpdateHorizontalMotion(
    UGameMovementComponent* Movement,
    UMotionComponent* Motion,
    const FVector& DesiredDirection,
    float TargetSpeed,
    float DeltaTime)
{
    if (!Movement || !Motion)
    {
        return;
    }

    /*
     * MotionComponent is the owner of actual velocity.
     */
    const FVector CurrentVelocity =
        Motion->Velocity;

    /*
     * Horizontal movement is controlled here.
     *
     * Vertical velocity belongs to gravity/jump/physics.
     */
    FVector HorizontalVelocity(
        CurrentVelocity.X,
        CurrentVelocity.Y,
        0.0f
    );

    const FVector TargetVelocity =
        DesiredDirection * TargetSpeed;

    const bool bHasMovement =
        !DesiredDirection.IsNearlyZero();

    const float Rate =
        bHasMovement
            ? Movement->Acceleration
            : Movement->Deceleration;

    /*
     * Airborne movement has reduced control.
     */
    const float EffectiveRate =
        Movement->IsGrounded()
            ? Rate
            : Movement->AirAcceleration;

    HorizontalVelocity =
        FMath::VInterpConstantTo(
            HorizontalVelocity,
            TargetVelocity,
            DeltaTime,
            EffectiveRate
        );

    /*
     * Preserve vertical velocity.
     */
    Motion->Velocity =
        FVector(
            HorizontalVelocity.X,
            HorizontalVelocity.Y,
            CurrentVelocity.Z
        );

    /*
     * DesiredVelocity represents what the movement system
     * requested.
     *
     * Velocity represents what the Actor is actually doing.
     */
    Motion->DesiredVelocity =
        FVector(
            TargetVelocity.X,
            TargetVelocity.Y,
            CurrentVelocity.Z
        );
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
     * Jump modifies actual motion.
     *
     * Gravity/physics can subsequently take ownership of Z.
     */
    Motion->Velocity.Z =
        Movement->JumpVelocity;

    Movement->SetGrounded(false);

    Movement->SetMovementMode(
        EGameMovementMode::Airborne
    );
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
