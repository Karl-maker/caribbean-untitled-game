//
// MovementAnimationSystem.cpp
// CaribbeanUntitled
//

#include "MovementAnimationSystem.h"

#include "AnimationSystem.h"
#include "../EventBus/EventBusSubsystem.h"
#include "../Events/MovementPayload.h" // FMovementPayload, EControlIntent (via ControlComponent.h)

#include "Engine/World.h"


//=============================================================================
// USubsystem
//=============================================================================

void UMovementAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Subscribe<FMovementPayload>(
            this,
            &UMovementAnimationSystem::OnMovementControlEvent
        );
    }
}

void UMovementAnimationSystem::Deinitialize()
{
    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Unsubscribe(this);
    }

    Super::Deinitialize();
}


//=============================================================================
// Lookups
//=============================================================================

UEventBusSubsystem* UMovementAnimationSystem::GetEventBus() const
{
    const UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UEventBusSubsystem>() : nullptr;
}

UAnimationSystem* UMovementAnimationSystem::GetAnimationSystem() const
{
    const UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UAnimationSystem>() : nullptr;
}


//=============================================================================
// Translation
//=============================================================================

void UMovementAnimationSystem::OnMovementControlEvent(const FMovementPayload& Payload)
{
    EAnimationKey Key;
    if (!MapControlIntentToAnimationKey(Payload.Type, Key))
    {
        return;
    }

    if (UAnimationSystem* AnimationSystem = GetAnimationSystem())
    {
        AnimationSystem->RequestAnimationState(Payload.Source, Key);
    }
}

bool UMovementAnimationSystem::MapControlIntentToAnimationKey(
    ELocomotion Movement,
    EAnimationKey& OutKey
) const
{
    switch (Movement)
    {
        case ELocomotion::Walk:
            OutKey = DefaultLocomotionKey;
            return true;

        case ELocomotion::Sprint:
            OutKey = EAnimationKey::Sprint;
            return true;

        case ELocomotion::Jump:
            OutKey = EAnimationKey::Jump;
            return true;


        default:
            return false;
    }
}
