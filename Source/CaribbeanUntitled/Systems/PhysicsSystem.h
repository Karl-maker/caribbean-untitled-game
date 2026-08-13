//
// PhysicsSystem.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "PhysicsSystem.generated.h"

class URigidbodyComponent;
class UMotionComponent;

/**
 * Central physics system for the game world.
 *
 * This subsystem is responsible for coordinating physics logic.
 *
 * RigidbodyComponent owns the actual physics body/interface.
 * PhysicsSystem owns the physics update logic.
 *
 * MovementSystem -> MotionComponent -> PhysicsSystem
 *                                  -> RigidbodyComponent -> Chaos
 */
UCLASS()
class CARIBBEANUNTITLED_API UPhysicsSystem
    : public UWorldSubsystem
    , public FTickableGameObject
{
    GENERATED_BODY()

public:

    // ========================================================================
    // SUBSYSTEM
    // ========================================================================

    virtual void Initialize(
        FSubsystemCollectionBase& Collection) override;

    virtual void Deinitialize() override;


    // ========================================================================
    // REGISTRATION
    // ========================================================================

    void RegisterRigidbody(
        URigidbodyComponent* Rigidbody);

    void UnregisterRigidbody(
        URigidbodyComponent* Rigidbody);


private:

    // ========================================================================
    // PHYSICS PIPELINE
    // ========================================================================

    void SimulatePhysics(
        float DeltaTime);

    void ProcessMotion(
        URigidbodyComponent* Rigidbody,
        UMotionComponent* Motion,
        float DeltaTime);

    void ProcessJump(
        URigidbodyComponent* Rigidbody,
        UMotionComponent* Motion);

    void UpdateGroundedState(
        URigidbodyComponent* Rigidbody);

    void UpdatePhysicsState(
        URigidbodyComponent* Rigidbody);

    void CleanupRegistry();
    
    virtual void Tick(float DeltaTime) override;

    virtual bool IsTickable() const override;

    virtual TStatId GetStatId() const override;


private:

    // ========================================================================
    // REGISTRY
    // ========================================================================

    TArray<TWeakObjectPtr<URigidbodyComponent>> Rigidbodies;

    TSet<TWeakObjectPtr<URigidbodyComponent>> RigidbodySet;


private:

    // ========================================================================
    // CONFIGURATION
    // ========================================================================

    UPROPERTY()
    float MovementAcceleration = 5000.0f;

    UPROPERTY()
    float AirAcceleration = 1500.0f;

    UPROPERTY()
    float JumpImpulse = 42000.0f;

    UPROPERTY()
    float GroundNormalThreshold = 0.7f;

    UPROPERTY()
    float GroundCheckDistance = 8.0f;
};
