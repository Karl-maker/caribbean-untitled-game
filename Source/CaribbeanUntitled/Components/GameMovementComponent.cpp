//
//  MovementComponent.cpp
//  CaribbeanUntitled
//
//  Created by Karl-Johan Bailey on 8/12/26.
//

#include "GameMovementComponent.h"


UGameMovementComponent::UGameMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    DesiredDirection = FVector::ZeroVector;
    DesiredVelocity = FVector::ZeroVector;
    Velocity = FVector::ZeroVector;

    MovementMode = EGameMovementMode::Grounded;

    bIsGrounded = true;
    bIsSprinting = false;
    bIsCrouching = false;
    bIsDashing = false;

    DashTimeRemaining = 0.0f;
    DashCooldownRemaining = 0.0f;
}


void UGameMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    DesiredDirection = FVector::ZeroVector;
    DesiredVelocity = FVector::ZeroVector;
}


FVector UGameMovementComponent::GetDesiredDirection() const
{
    return DesiredDirection;
}


FVector UGameMovementComponent::GetDesiredVelocity() const
{
    return DesiredVelocity;
}


FVector UGameMovementComponent::GetVelocity() const
{
    return Velocity;
}


EGameMovementMode UGameMovementComponent::GetMovementMode() const
{
    return MovementMode;
}


bool UGameMovementComponent::IsGrounded() const
{
    return bIsGrounded;
}


bool UGameMovementComponent::IsSprinting() const
{
    return bIsSprinting;
}


bool UGameMovementComponent::IsCrouching() const
{
    return bIsCrouching;
}


bool UGameMovementComponent::IsDashing() const
{
    return bIsDashing;
}


void UGameMovementComponent::SetDesiredDirection(
    const FVector& Direction
)
{
    if (Direction.IsNearlyZero())
    {
        DesiredDirection = FVector::ZeroVector;
        return;
    }

    DesiredDirection = Direction.GetSafeNormal();
}


void UGameMovementComponent::SetDesiredVelocity(
    const FVector& NewVelocity
)
{
    DesiredVelocity = NewVelocity;
}


void UGameMovementComponent::SetVelocity(
    const FVector& NewVelocity
)
{
    Velocity = NewVelocity;
}


void UGameMovementComponent::SetMovementMode(
    EGameMovementMode NewMode
)
{
    MovementMode = NewMode;

    switch (NewMode)
    {
        case EGameMovementMode::Grounded:
            bIsGrounded = true;
            break;

        case EGameMovementMode::Airborne:
            bIsGrounded = false;
            break;

        case EGameMovementMode::Crouching:
            bIsCrouching = true;
            break;

        case EGameMovementMode::Sprinting:
            bIsSprinting = true;
            break;

        case EGameMovementMode::Dashing:
            bIsDashing = true;
            break;

        case EGameMovementMode::Swimming:
            bIsGrounded = false;
            break;

        default:
            break;
    }
}


void UGameMovementComponent::SetGrounded(bool bGrounded)
{
    bIsGrounded = bGrounded;

    if (!bGrounded &&
        MovementMode == EGameMovementMode::Grounded)
    {
        MovementMode = EGameMovementMode::Airborne;
    }

    if (bGrounded &&
        MovementMode == EGameMovementMode::Airborne)
    {
        MovementMode = EGameMovementMode::Grounded;
    }
}


void UGameMovementComponent::ClearDesiredMovement()
{
    DesiredDirection = FVector::ZeroVector;
    DesiredVelocity = FVector::ZeroVector;
}


void UGameMovementComponent::SetSprinting(bool bSprinting)
{
    bIsSprinting = bSprinting;

    if (bSprinting)
    {
        MovementMode = EGameMovementMode::Sprinting;
    }
    else if (MovementMode == EGameMovementMode::Sprinting)
    {
        MovementMode = bIsGrounded
            ? EGameMovementMode::Grounded
            : EGameMovementMode::Airborne;
    }
}


void UGameMovementComponent::SetCrouching(bool bCrouching)
{
    bIsCrouching = bCrouching;

    if (bCrouching)
    {
        MovementMode = EGameMovementMode::Crouching;
    }
    else if (MovementMode == EGameMovementMode::Crouching)
    {
        MovementMode = bIsGrounded
            ? EGameMovementMode::Grounded
            : EGameMovementMode::Airborne;
    }
}


void UGameMovementComponent::SetDashing(bool bDashing)
{
    bIsDashing = bDashing;

    if (bDashing)
    {
        MovementMode = EGameMovementMode::Dashing;
    }
    else if (MovementMode == EGameMovementMode::Dashing)
    {
        MovementMode = bIsGrounded
            ? EGameMovementMode::Grounded
            : EGameMovementMode::Airborne;
    }
}


bool UGameMovementComponent::CanDash() const
{
    return !bIsDashing &&
           DashCooldownRemaining <= KINDA_SMALL_NUMBER;
}


void UGameMovementComponent::StartDash()
{
    if (!CanDash())
    {
        return;
    }

    bIsDashing = true;
    MovementMode = EGameMovementMode::Dashing;

    DashTimeRemaining = DashDuration;
}


void UGameMovementComponent::StopDash()
{
    bIsDashing = false;

    DashTimeRemaining = 0.0f;
    DashCooldownRemaining = DashCooldown;

    MovementMode = bIsGrounded
        ? EGameMovementMode::Grounded
        : EGameMovementMode::Airborne;
}


void UGameMovementComponent::SetDashCooldown(float RemainingTime)
{
    DashCooldownRemaining = FMath::Max(
        RemainingTime,
        0.0f
    );
}


float UGameMovementComponent::GetCurrentMaxSpeed() const
{
    if (bIsDashing)
    {
        return DashSpeed;
    }

    if (bIsCrouching)
    {
        return CrouchSpeed;
    }

    if (bIsSprinting)
    {
        return SprintSpeed;
    }

    return WalkSpeed;
}
