#include "MotionComponent.h"

UMotionComponent::UMotionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    Velocity = FVector::ZeroVector;
    DesiredVelocity = FVector::ZeroVector;
    Acceleration = FVector::ZeroVector;

    PreviousVelocity = FVector::ZeroVector;
    Direction = FVector::ForwardVector;
    Speed = 0.0f;

    AngularVelocity = FVector::ZeroVector;
    DesiredAngularVelocity = FVector::ZeroVector;
    AngularAcceleration = FVector::ZeroVector;
}

void UMotionComponent::BeginPlay()
{
    Super::BeginPlay();

    UpdateDerivedState();
}

void UMotionComponent::UpdateDerivedState()
{
    Speed = Velocity.Size();

    if (!Velocity.IsNearlyZero())
    {
        Direction = Velocity.GetSafeNormal();
    }
}

void UMotionComponent::SavePreviousState()
{
    PreviousVelocity = Velocity;
}

bool UMotionComponent::IsMoving() const
{
    return !Velocity.IsNearlyZero();
}

bool UMotionComponent::IsMovingAboveSpeed(float Threshold) const
{
    return Speed > Threshold;
}
