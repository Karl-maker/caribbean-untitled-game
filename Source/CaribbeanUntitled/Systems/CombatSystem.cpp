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


    SubscribeToEvents();
}


void UCombatSystem::Deinitialize()
{
    UnsubscribeFromEvents();


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

        return;
    }

    EventBus->Subscribe<FCombatCollisionPayload>(
        this,
        &UCombatSystem::OnCombatCollision
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
}
