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
    EControlIntent Intent,
    EAnimationKey& OutKey
) const
{
    switch (Intent)
    {
        case EControlIntent::MoveForward:
        case EControlIntent::MoveBack:
        case EControlIntent::MoveLeft:
        case EControlIntent::MoveRight:
            OutKey = DefaultLocomotionKey;
            return true;

        case EControlIntent::Sprint:
            OutKey = EAnimationKey::Sprint;
            return true;

        case EControlIntent::Jump:
            OutKey = EAnimationKey::Jump;
            return true;

        case EControlIntent::Interact:
            OutKey = EAnimationKey::Interact;
            return true;

        // No EAnimationKey equivalent yet — add one and a case here
        // once these are ready to animate.
        case EControlIntent::Crouch:
        case EControlIntent::Dash:
        case EControlIntent::Aim:
        case EControlIntent::Fire:
        case EControlIntent::Reload:
        default:
            return false;
    }
}
