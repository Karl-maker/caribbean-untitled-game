//
// CombatSystem.cpp
// CaribbeanUntitled
//

#include "CombatSystem.h"

#include "../EventBus/EventBusSubsystem.h"
#include "../Components/CombatVolumeComponent.h"
#include "CombatDetectionSystem.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"


//=============================================================================
// Lifecycle
//=============================================================================

void UCombatSystem::Initialize(
    FSubsystemCollectionBase& Collection
)
{
    Super::Initialize(Collection);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("CombatSystem initialized.")
    );

    SubscribeToEvents();
}


void UCombatSystem::Deinitialize()
{
    UnsubscribeFromEvents();

    UE_LOG(
        LogTemp,
        Log,
        TEXT("CombatSystem deinitialized.")
    );

    Super::Deinitialize();
}


//=============================================================================
// Event Bus
//=============================================================================

UEventBusSubsystem* UCombatSystem::GetEventBus() const
{
    const UWorld* World = GetWorld();

    if (!World)
    {
        return nullptr;
    }

    return World->GetSubsystem<UEventBusSubsystem>();
}


void UCombatSystem::SubscribeToEvents()
{
    UEventBusSubsystem* EventBus = GetEventBus();

    if (!EventBus)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("CombatSystem::SubscribeToEvents - EventBus not found.")
        );

        return;
    }

    EventBus->Subscribe<FCombatCollisionPayload>(
        this,
        &UCombatSystem::OnCombatCollision
    );

    UE_LOG(
        LogTemp,
        Log,
        TEXT("CombatSystem subscribed to FCombatCollisionPayload.")
    );
}


void UCombatSystem::UnsubscribeFromEvents()
{
    UEventBusSubsystem* EventBus = GetEventBus();

    if (!EventBus)
    {
        return;
    }

    EventBus->Unsubscribe(this);
}


//=============================================================================
// Combat Events
//=============================================================================

void UCombatSystem::OnCombatCollision(
    const FCombatCollisionPayload& Payload
)
{
    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "Combat Collision | "
            "SourceActor: %s | "
            "TargetActor: %s | "
            "SourceVolume: %s | "
            "TargetVolume: %s | "
            "TargetComponent: %s | "
            "SourceAttachedTo: %s | "
            "Socket: %s | "
            "Location: %s"
        ),
        IsValid(Payload.SourceActor)
            ? *Payload.SourceActor->GetName()
            : TEXT("None"),

        IsValid(Payload.TargetActor)
            ? *Payload.TargetActor->GetName()
            : TEXT("None"),

        IsValid(Payload.SourceVolume)
            ? *Payload.SourceVolume->GetName()
            : TEXT("None"),

        IsValid(Payload.TargetVolume)
            ? *Payload.TargetVolume->GetName()
            : TEXT("None"),

        IsValid(Payload.TargetComponent)
            ? *Payload.TargetComponent->GetName()
            : TEXT("None"),

        IsValid(Payload.SourceAttachedTo)
            ? *Payload.SourceAttachedTo->GetName()
            : TEXT("None"),

        *Payload.SourceAttachSocket.ToString(),

        *Payload.Location.ToString()
    );
}
