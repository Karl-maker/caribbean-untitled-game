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

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("MovementAnimationSystem INITIALIZED - Instance: %p - World: %s"),
        this,
        *GetNameSafe(GetWorld())
    );

    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MovementAnimationSystem SUBSCRIBING - Instance: %p"),
            this
        );

        EventBus->Subscribe<FMovementPayload>(
            this,
            &UMovementAnimationSystem::OnMovementControlEvent
        );
    }
}

void UMovementAnimationSystem::Deinitialize()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("MovementAnimationSystem DEINITIALIZED - Instance: %p - World: %s"),
        this,
        *GetNameSafe(GetWorld())
    );

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

void UMovementAnimationSystem::OnMovementControlEvent(
    const FMovementPayload& Payload
)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("MovementAnimationSystem EVENT - Instance: %p - Source: %s - Movement: %d"),
        this,
        *GetNameSafe(Payload.Source),
        static_cast<uint8>(Payload.Type)
    );

    EAnimationKey Key;

    if (!MapControlIntentToAnimationKey(Payload.Type, Key))
    {
        return;
    }

    if (UAnimationSystem* AnimationSystem = GetAnimationSystem())
    {
        AnimationSystem->RequestAnimationState(
            Payload.Source,
            Key
        );
        
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MovementAnimationSystem TRIGGERED EVENT FOR ANIMATION REQUEST")
        );
    }
}

bool UMovementAnimationSystem::MapControlIntentToAnimationKey(
    ELocomotion Movement,
    EAnimationKey& OutKey
) const
{
    UE_LOG(
        LogTemp,
        Log,
        TEXT("MovementAnimationSystem: Movement = %d"),
        static_cast<uint8>(Movement)
    );

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

        case ELocomotion::Land:
            OutKey = EAnimationKey::Land;
            return true;

        case ELocomotion::Fall:
            OutKey = EAnimationKey::Fall;
            return true;
            
        case ELocomotion::Idle:
            OutKey = EAnimationKey::Idle;
            return true;

        default:
            UE_LOG(LogTemp, Warning, TEXT("MovementAnimationSystem: Unknown locomotion value = %d"),
                static_cast<uint8>(Movement));
            return false;
    }
}
