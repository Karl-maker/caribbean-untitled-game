//
// AnimationComponent.cpp
// CaribbeanUntitled
//

#include "AnimationComponent.h"


UAnimationComponent::UAnimationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


//=============================================================================
// Animation Lookup - C++ Only
//=============================================================================

const FAnimationDefinition* UAnimationComponent::FindAnimation(EAnimationKey Key) const
{
    for (const FAnimationDefinition& Anim : Animations)
    {
        if (Anim.Key == Key)
            return &Anim;
    }
    return nullptr;
}


//=============================================================================
// Animation Lookup - Blueprint Safe
//=============================================================================

FAnimationDefinition UAnimationComponent::GetAnimation(EAnimationKey Key) const
{
    if (const FAnimationDefinition* Found = FindAnimation(Key))
    {
        return *Found;
    }
    return FAnimationDefinition();
}


bool UAnimationComponent::HasAnimation(EAnimationKey Key) const
{
    return FindAnimation(Key) != nullptr;
}


bool UAnimationComponent::TryFindAnimation(
    EAnimationKey Key,
    FAnimationDefinition& OutAnimation) const
{
    if (const FAnimationDefinition* Found = FindAnimation(Key))
    {
        OutAnimation = *Found;
        return true;
    }
    OutAnimation = FAnimationDefinition();
    return false;
}


//=============================================================================
// Animation Requests
//=============================================================================

void UAnimationComponent::RequestAnimation(EAnimationKey Key)
{
    if (Key == EAnimationKey::None)
    {
        CurrentAnimationKey = EAnimationKey::None;
        return;
    }

    if (!HasAnimation(Key))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("AnimationComponent on %s has no animation for key %d"),
            *GetOwner()->GetName(),
            static_cast<uint8>(Key)
        );

        return;
    }

    CurrentAnimationKey = Key;
}


void UAnimationComponent::ClearAnimationRequest()
{
    CurrentAnimationKey = EAnimationKey::None;
}


//=============================================================================
// State Management
//=============================================================================

void UAnimationComponent::SetActiveAnimation(EAnimationKey Key)
{
    ActiveAnimationKey = Key;
}


void UAnimationComponent::SetBlending(bool bBlending)
{
    bIsBlending = bBlending;
}
