//
// AnimationRegistryComponent.cpp
// CaribbeanUntitled
//

#include "AnimationRegistryComponent.h"


// ============================================================================
// Constructor
// ============================================================================

UAnimationRegistryComponent::UAnimationRegistryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


// ============================================================================
// Animation Lookup
// ============================================================================

const FAnimationDefinition*
UAnimationRegistryComponent::FindAnimation(
    EAnimationKey Key
) const
{
    for (const FAnimationDefinition& Definition : Animations)
    {
        if (Definition.Key == Key)
        {
            return &Definition;
        }
    }

    return nullptr;
}


FAnimationDefinition
UAnimationRegistryComponent::GetAnimation(
    EAnimationKey Key
) const
{
    const FAnimationDefinition* Definition =
        FindAnimation(Key);

    if (Definition)
    {
        return *Definition;
    }

    return FAnimationDefinition();
}


bool UAnimationRegistryComponent::HasAnimation(
    EAnimationKey Key
) const
{
    return FindAnimation(Key) != nullptr;
}


bool UAnimationRegistryComponent::TryFindAnimation(
    EAnimationKey Key,
    FAnimationDefinition& OutAnimation
) const
{
    const FAnimationDefinition* Definition =
        FindAnimation(Key);

    if (!Definition)
    {
        OutAnimation = FAnimationDefinition();
        return false;
    }

    OutAnimation = *Definition;

    return true;
}
