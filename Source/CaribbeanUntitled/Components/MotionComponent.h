#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MotionComponent.generated.h"

/**
 * Stores the current and desired motion state of an Actor.
 *
 * Movement systems modify DesiredVelocity / Acceleration.
 * Physics systems modify Velocity / AngularVelocity.
 * Transform systems consume the resulting motion.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UMotionComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UMotionComponent();

    // -------------------------------------------------------------------------
    // Linear Motion
    // -------------------------------------------------------------------------

    /** Current world-space velocity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Linear")
    FVector Velocity = FVector::ZeroVector;

    /** Desired world-space velocity requested by movement/AI/input systems. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Linear")
    FVector DesiredVelocity = FVector::ZeroVector;

    /** Current world-space acceleration. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Linear")
    FVector Acceleration = FVector::ZeroVector;

    /** Velocity from the previous motion update. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Motion|Linear")
    FVector PreviousVelocity = FVector::ZeroVector;

    /** Direction of current movement. Derived from Velocity. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Motion|Linear")
    FVector Direction = FVector::ForwardVector;

    /** Magnitude of current velocity. Derived from Velocity. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Motion|Linear")
    float Speed = 0.0f;


    // -------------------------------------------------------------------------
    // Angular Motion
    // -------------------------------------------------------------------------

    /** Current angular velocity in degrees per second. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Angular")
    FVector AngularVelocity = FVector::ZeroVector;

    /** Desired angular velocity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Angular")
    FVector DesiredAngularVelocity = FVector::ZeroVector;

    /** Current angular acceleration. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion|Angular")
    FVector AngularAcceleration = FVector::ZeroVector;


    // -------------------------------------------------------------------------
    // State
    // -------------------------------------------------------------------------

    /** Updates derived motion values such as Speed and Direction. */
    UFUNCTION(BlueprintCallable, Category="Motion")
    void UpdateDerivedState();

    /** Saves the current velocity as the previous velocity. */
    UFUNCTION(BlueprintCallable, Category="Motion")
    void SavePreviousState();

    /** Returns whether the Actor is currently moving. */
    UFUNCTION(BlueprintPure, Category="Motion")
    bool IsMoving() const;

    /** Returns whether the Actor is moving faster than the supplied threshold. */
    UFUNCTION(BlueprintPure, Category="Motion")
    bool IsMovingAboveSpeed(float Threshold) const;

protected:

    virtual void BeginPlay() override;
};
