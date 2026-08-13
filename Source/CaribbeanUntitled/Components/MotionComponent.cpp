#include "MotionComponent.h"

UMotionComponent::UMotionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // -------------------------------------------------------------------------
    // Linear Motion Request
    // -------------------------------------------------------------------------

    DesiredVelocity = FVector::ZeroVector;
    DesiredAcceleration = FVector::ZeroVector;

    DesiredDirection = FVector::ForwardVector;
    DesiredSpeed = 0.0f;

    // -------------------------------------------------------------------------
    // Derived State
    // -------------------------------------------------------------------------

    Direction = FVector::ForwardVector;
    Speed = 0.0f;

    // -------------------------------------------------------------------------
    // Angular Motion Request
    // -------------------------------------------------------------------------

    DesiredAngularVelocity = FVector::ZeroVector;
    DesiredAngularAcceleration = FVector::ZeroVector;
    
    bWantsToJump = false;
}


void UMotionComponent::BeginPlay()
{
    Super::BeginPlay();

    UpdateDerivedState();
}


void UMotionComponent::UpdateDerivedState()
{
    // -------------------------------------------------------------------------
    // Desired Linear Motion
    // -------------------------------------------------------------------------

    DesiredSpeed = DesiredVelocity.Size();

    if (!DesiredVelocity.IsNearlyZero())
    {
        DesiredDirection =
            DesiredVelocity.GetSafeNormal();
    }
    else
    {
        DesiredDirection = FVector::ZeroVector;
    }


    // -------------------------------------------------------------------------
    // Derived Movement State
    // -------------------------------------------------------------------------

    Speed = DesiredSpeed;

    if (!DesiredVelocity.IsNearlyZero())
    {
        Direction =
            DesiredVelocity.GetSafeNormal();
    }
    else
    {
        Direction = FVector::ZeroVector;
    }
}


void UMotionComponent::ClearMotionRequest()
{
    DesiredVelocity = FVector::ZeroVector;
    DesiredAcceleration = FVector::ZeroVector;

    DesiredDirection = FVector::ZeroVector;
    DesiredSpeed = 0.0f;

    DesiredAngularVelocity = FVector::ZeroVector;
    DesiredAngularAcceleration = FVector::ZeroVector;

    Speed = 0.0f;
    Direction = FVector::ZeroVector;
    
    bWantsToJump = false;
}


bool UMotionComponent::IsMoving() const
{
    return !DesiredVelocity.IsNearlyZero();
}


bool UMotionComponent::IsMovingAboveSpeed(float Threshold) const
{
    return DesiredSpeed > Threshold;
}
