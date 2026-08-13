//
//  ControlComponent.cpp
//  CaribbeanUntitled
//
//  Created by Karl-Johan Bailey on 8/12/26.
//

#include "ControlComponent.h"


UControlComponent::UControlComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    MoveIntent = FVector2D::ZeroVector;
    JumpIntent = 0.0f;
    InteractIntent = 0.0f;
}


void UControlComponent::BeginPlay()
{
    Super::BeginPlay();

    ClearIntents();
}


void UControlComponent::SetMoveIntent(const FVector2D& Intent)
{
    MoveIntent.X = FMath::Clamp(Intent.X, -1.0f, 1.0f);
    MoveIntent.Y = FMath::Clamp(Intent.Y, -1.0f, 1.0f);
}


void UControlComponent::SetForwardIntent(float Weight)
{
    MoveIntent.Y = FMath::Clamp(Weight, 0.0f, 1.0f);
}


void UControlComponent::SetBackIntent(float Weight)
{
    MoveIntent.Y = -FMath::Clamp(Weight, 0.0f, 1.0f);
}


void UControlComponent::SetLeftIntent(float Weight)
{
    MoveIntent.X = -FMath::Clamp(Weight, 0.0f, 1.0f);
}


void UControlComponent::SetRightIntent(float Weight)
{
    MoveIntent.X = FMath::Clamp(Weight, 0.0f, 1.0f);
}


void UControlComponent::SetJumpIntent(float Weight)
{
    JumpIntent = FMath::Clamp(Weight, 0.0f, 1.0f);
}


void UControlComponent::SetInteractIntent(float Weight)
{
    InteractIntent = FMath::Clamp(Weight, 0.0f, 1.0f);
}


float UControlComponent::GetIntentWeight(
    EControlIntent Intent
) const
{
    switch (Intent)
    {
        case EControlIntent::Forward:
            return FMath::Max(MoveIntent.Y, 0.0f);

        case EControlIntent::Back:
            return FMath::Max(-MoveIntent.Y, 0.0f);

        case EControlIntent::Left:
            return FMath::Max(-MoveIntent.X, 0.0f);

        case EControlIntent::Right:
            return FMath::Max(MoveIntent.X, 0.0f);

        case EControlIntent::Jump:
            return JumpIntent;

        case EControlIntent::Interact:
            return InteractIntent;

        default:
            return 0.0f;
    }
}


void UControlComponent::SetIntentWeight(
    EControlIntent Intent,
    float Weight
)
{
    Weight = FMath::Clamp(Weight, 0.0f, 1.0f);

    switch (Intent)
    {
        case EControlIntent::Forward:
            SetForwardIntent(Weight);
            break;

        case EControlIntent::Back:
            SetBackIntent(Weight);
            break;

        case EControlIntent::Left:
            SetLeftIntent(Weight);
            break;

        case EControlIntent::Right:
            SetRightIntent(Weight);
            break;

        case EControlIntent::Jump:
            SetJumpIntent(Weight);
            break;

        case EControlIntent::Interact:
            SetInteractIntent(Weight);
            break;

        default:
            break;
    }
}


bool UControlComponent::HasIntent(
    EControlIntent Intent,
    float MinimumWeight
) const
{
    return GetIntentWeight(Intent) >= MinimumWeight;
}


bool UControlComponent::HasMovementIntent() const
{
    return !MoveIntent.IsNearlyZero();
}


bool UControlComponent::HasActionIntent() const
{
    return JumpIntent > KINDA_SMALL_NUMBER ||
           InteractIntent > KINDA_SMALL_NUMBER;
}


void UControlComponent::ClearMovementIntent()
{
    MoveIntent = FVector2D::ZeroVector;
}


void UControlComponent::ClearActionIntent()
{
    JumpIntent = 0.0f;
    InteractIntent = 0.0f;
}


void UControlComponent::ClearIntents()
{
    ClearMovementIntent();
    ClearActionIntent();
}
