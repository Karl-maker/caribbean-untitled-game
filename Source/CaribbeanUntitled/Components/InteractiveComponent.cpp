//
// InteractiveComponent.cpp
// CaribbeanUntitled
//

#include "InteractiveComponent.h"

#include "GameFramework/Actor.h"


UInteractiveComponent::UInteractiveComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


//=============================================================================
// Lifecycle
//=============================================================================

void UInteractiveComponent::BeginPlay()
{
    Super::BeginPlay();

    ClearInteraction();
}


//=============================================================================
// Interaction Requests
//=============================================================================

void UInteractiveComponent::RequestInteraction(
    EInteractiveType Type,
    AActor* Target
)
{
    if (!IsValid(Target))
    {
        ClearInteraction();
        return;
    }

    InteractionType = Type;
    InteractionTarget = Target;
    bInteractionRequested = true;

    ResetInteractionProgress();
}


void UInteractiveComponent::ClearInteraction()
{
    bInteractionRequested = false;
    InteractionType = EInteractiveType::None;
    InteractionTarget = nullptr;

    ResetInteractionProgress();
}


void UInteractiveComponent::SetInteractionTarget(AActor* Target)
{
    InteractionTarget = Target;

    if (!IsValid(Target))
    {
        bInteractionRequested = false;
        InteractionType = EInteractiveType::None;
        ResetInteractionProgress();
    }
}


void UInteractiveComponent::SetInteractionType(EInteractiveType Type)
{
    InteractionType = Type;

    if (Type == EInteractiveType::None)
    {
        bInteractionRequested = false;
        ResetInteractionProgress();
    }
}


//=============================================================================
// Range
//=============================================================================

void UInteractiveComponent::SetInteractionRange(float Range)
{
    InteractionRange = FMath::Max(0.0f, Range);
}


bool UInteractiveComponent::IsTargetInRange() const
{
    AActor* Owner = GetOwner();

    if (!IsValid(Owner) || !IsValid(InteractionTarget))
    {
        return false;
    }

    const float DistanceSquared =
        FVector::DistSquared(
            Owner->GetActorLocation(),
            InteractionTarget->GetActorLocation()
        );

    return DistanceSquared <= FMath::Square(InteractionRange);
}


//=============================================================================
// Progress
//=============================================================================

void UInteractiveComponent::SetInteractionProgress(float Progress)
{
    InteractionProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
}


void UInteractiveComponent::ResetInteractionProgress()
{
    InteractionProgress = 0.0f;
}
