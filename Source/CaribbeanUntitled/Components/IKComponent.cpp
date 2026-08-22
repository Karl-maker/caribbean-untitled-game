//
// IKComponent.cpp
// CaribbeanUntitled
//

#include "IKComponent.h"

UIKComponent::UIKComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UIKComponent::SetTarget(FName Channel, const FIKTargetData& Target)
{
    const int32 Index = FindTargetIndex(Channel);
    if (Index != INDEX_NONE)
    {
        Targets[Index].Target = Target;
        return;
    }

    FIKTargetEntry& Entry = Targets.AddDefaulted_GetRef();
    Entry.Channel = Channel;
    Entry.Target = Target;
}

void UIKComponent::ClearTarget(FName Channel)
{
    const int32 Index = FindTargetIndex(Channel);
    if (Index != INDEX_NONE)
    {
        Targets.RemoveAtSwap(Index);
    }
}

void UIKComponent::SetBoneChain(FName Channel, FName InBoneName, FName InParentBoneName)
{
    const int32 Index = FindTargetIndex(Channel);
    if (Index == INDEX_NONE)
    {
        FIKTargetEntry& Entry = Targets.AddDefaulted_GetRef();
        Entry.Channel = Channel;
        Entry.Target.BoneName = InBoneName;
        Entry.Target.ParentBoneName = InParentBoneName;
        Entry.Target.bEnabled = true;
        return;
    }

    Targets[Index].Target.BoneName = InBoneName;
    Targets[Index].Target.ParentBoneName = InParentBoneName;
    Targets[Index].Target.bEnabled = true;
}

bool UIKComponent::GetTarget(FName Channel, FIKTargetData& OutTarget) const
{
    const int32 Index = FindTargetIndex(Channel);
    if (Index != INDEX_NONE)
    {
        OutTarget = Targets[Index].Target;
        return true;
    }
    return false;
}

const FIKTargetData* UIKComponent::FindTarget(FName Channel) const
{
    const int32 Index = FindTargetIndex(Channel);
    return Index != INDEX_NONE ? &Targets[Index].Target : nullptr;
}

bool UIKComponent::HasTarget(FName Channel) const
{
    return FindTargetIndex(Channel) != INDEX_NONE;
}

int32 UIKComponent::FindTargetIndex(FName Channel) const
{
    return Targets.IndexOfByPredicate([Channel](const FIKTargetEntry& Entry)
    {
        return Entry.Channel == Channel;
    });
}

void UIKComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UIKComponent, Targets);
}
