#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MotionComponent.generated.h"

/**
 * Stores requested motion and derived movement state for an Actor.
 *
 * Movement systems write DesiredVelocity / DesiredAngularVelocity.
 *
 * Physics systems consume those requests and produce the actual
 * physical motion through UPhysicsComponent / UE Chaos.
 *
 * MotionComponent does NOT own actual velocity.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UMotionComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UMotionComponent();


    // -------------------------------------------------------------------------
    // Linear Motion Request
    // -------------------------------------------------------------------------

    /**
     * World-space velocity requested by movement, AI, input, etc.
     *
     * This is an INTENT, not the actual physical velocity.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Linear")
    FVector DesiredVelocity = FVector::ZeroVector;


    /**
     * Desired world-space acceleration requested by gameplay systems.
     *
     * Optional for systems that want to request acceleration directly.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Linear")
    FVector DesiredAcceleration = FVector::ZeroVector;


    /**
     * Direction requested by the movement system.
     *
     * Usually derived from DesiredVelocity.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Motion|Linear")
    FVector DesiredDirection = FVector::ForwardVector;


    /**
     * Requested movement speed.
     *
     * This is not necessarily the actual physical speed.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Motion|Linear")
    float DesiredSpeed = 0.0f;


    // -------------------------------------------------------------------------
    // Derived State
    // -------------------------------------------------------------------------

    /**
     * Direction of requested movement.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Motion|State")
    FVector Direction = FVector::ForwardVector;


    /**
     * Requested movement speed.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Motion|State")
    float Speed = 0.0f;


    // -------------------------------------------------------------------------
    // Angular Motion Request
    // -------------------------------------------------------------------------

    /**
     * Desired angular velocity in degrees per second.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Angular")
    FVector DesiredAngularVelocity = FVector::ZeroVector;


    /**
     * Desired angular acceleration.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Angular")
    FVector DesiredAngularAcceleration = FVector::ZeroVector;
    
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Intent")
    bool bWantsToJump = false;


    // -------------------------------------------------------------------------
    // State
    // -------------------------------------------------------------------------

    /**
     * Updates derived values from the current movement request.
     */
    UFUNCTION(BlueprintCallable, Category="Motion")
    void UpdateDerivedState();


    /**
     * Clears the current movement request.
     *
     * Useful at the beginning/end of a frame so stale input
     * does not continue affecting the actor.
     */
    UFUNCTION(BlueprintCallable, Category="Motion")
    void ClearMotionRequest();


    /**
     * Returns whether movement has been requested.
     */
    UFUNCTION(BlueprintPure, Category="Motion")
    bool IsMoving() const;


    /**
     * Returns whether requested movement is above the supplied speed.
     */
    UFUNCTION(BlueprintPure, Category="Motion")
    bool IsMovingAboveSpeed(float Threshold) const;


protected:

    virtual void BeginPlay() override;
};
