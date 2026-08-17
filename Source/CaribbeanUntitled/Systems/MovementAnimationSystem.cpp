//
// MovementAnimationSystem.cpp
// CaribbeanUntitled
//

#include "MovementAnimationSystem.h"

#include "AnimationSystem.h"
#include "../EventBus/EventBusSubsystem.h"
#include "../Events/MovementPayload.h"

#include "Engine/World.h"


void UMovementAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UEventBusSubsystem>();

    EventBus->Subscribe<FMovementPayload>(
        this,
        &UMovementAnimationSystem::OnMovementControlEvent
    );
}


void UMovementAnimationSystem::Deinitialize()
{
    UEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UEventBusSubsystem>();

    EventBus->Unsubscribe(this);

    Super::Deinitialize();
}


void UMovementAnimationSystem::OnMovementControlEvent(
    const FMovementPayload& Payload)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("ANIMATION RECEIVED -> %d"),
        static_cast<uint8>(Payload.Type)
    );

    EAnimationKey Key = EAnimationKey::Idle;

    switch (Payload.Type)
    {
        case ELocomotion::Idle:
            Key = EAnimationKey::Idle;
            break;

        case ELocomotion::Walk:
            Key = EAnimationKey::Walk;
            break;

        case ELocomotion::Sprint:
            Key = EAnimationKey::Sprint;
            break;

        case ELocomotion::Jump:
            Key = EAnimationKey::Jump;
            break;

        case ELocomotion::Land:
            Key = EAnimationKey::Land;
            break;

        case ELocomotion::Fall:
            Key = EAnimationKey::Fall;
            break;

        case ELocomotion::Crouch:
            Key = EAnimationKey::Crouch;
            break;
            
        default:
            Key = EAnimationKey::Idle;
            break;
    }

    GetWorld()
        ->GetSubsystem<UAnimationSystem>()
        ->RequestAnimationState(
            Payload.Source,
            Key
        );
}
