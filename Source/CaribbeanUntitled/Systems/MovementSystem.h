#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MovementSystem.generated.h"

class AGameActor;
class UControlComponent;
class UGameMovementComponent;
class UMotionComponent;

/**
 * Converts control intent into movement/motion.
 *
 * Pipeline:
 *
 * ControlComponent
 *      ↓
 * MovementSystem
 *      ↓
 * GameMovementComponent
 *      ↓
 * MotionComponent
 *
 * This system does NOT move Actors directly.
 */
UCLASS()
class CARIBBEANUNTITLED_API UMovementSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    /**
     * Processes all Actors that contain both:
     *
     * UControlComponent
     * UGameMovementComponent
     * UMotionComponent
     */
    void Tick(float DeltaTime);

protected:

    /**
     * Processes one Actor.
     */
    void ProcessActor(
        AGameActor* Actor,
        float DeltaTime
    );

    /**
     * Converts control input into a desired movement direction.
     */
    FVector CalculateDesiredDirection(
        AGameActor* Actor,
        const UControlComponent* Control
    ) const;

    /**
     * Calculates target movement speed based on
     * sprint/crouch state.
     */
    float CalculateTargetSpeed(
        const UGameMovementComponent* Movement,
        const UControlComponent* Control
    ) const;

    /**
     * Applies acceleration/deceleration to MotionComponent.
     */
    void UpdateHorizontalMotion(
        UGameMovementComponent* Movement,
        UMotionComponent* Motion,
        const FVector& DesiredDirection,
        float TargetSpeed,
        float DeltaTime
    );

    /**
     * Processes jumping.
     */
    void ProcessJump(
        UGameMovementComponent* Movement,
        UMotionComponent* Motion,
        const UControlComponent* Control
    );

    /**
     * Processes sprint state.
     */
    void ProcessSprint(
        UGameMovementComponent* Movement,
        const UControlComponent* Control
    );

    /**
     * Processes crouch state.
     */
    void ProcessCrouch(
        UGameMovementComponent* Movement,
        const UControlComponent* Control
    );
};
