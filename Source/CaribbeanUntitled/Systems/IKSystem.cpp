//
// IKSystem.cpp
// CaribbeanUntitled
//

#include "IKSystem.h"

#include "ActorRegistry.h"
#include "../Components/IKComponent.h"
#include "../EventBus/EventBusSubsystem.h"

#include "Engine/World.h"

TStatId UIKSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UIKSystem, STATGROUP_Tickables);
}

void UIKSystem::Cleanup()
{
    States.Reset();
    bSubscribed = false;
    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Unsubscribe(this);
    }
}

UEventBusSubsystem* UIKSystem::GetEventBus() const
{
    UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UEventBusSubsystem>() : nullptr;
}

UActorRegistry* UIKSystem::GetActorRegistry() const
{
    UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UActorRegistry>() : nullptr;
}

void UIKSystem::OnIKEvent(const FIKEventPayload& Payload)
{
    if (!IsValid(Payload.Source))
    {
        return;
    }

    UIKComponent* IK = Payload.Source->FindComponentByClass<UIKComponent>();
    if (!IK)
    {
        return;
    }

    if (Payload.Mode == EIKMode::Clear)
    {
        IK->ClearTarget(Payload.Channel);
        return;
    }

    IK->SetTarget(Payload.Channel, Payload.Target);
}

void UIKSystem::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    UEventBusSubsystem* EventBus = GetEventBus();
    if (EventBus && !bSubscribed)
    {
        EventBus->Subscribe<FIKEventPayload>(this, &UIKSystem::OnIKEvent);
        bSubscribed = true;
    }

    UActorRegistry* Registry = GetActorRegistry();
    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors = Registry->GetActorsWithComponent<UIKComponent>();
    if (!Actors)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *Actors)
    {
        AActor* Actor = WeakActor.Get();
        if (!IsValid(Actor))
        {
            continue;
        }

        FIKState& State = States.FindOrAdd(Actor);
        State.TimeUntilNextUpdate -= DeltaTime;
        if (State.TimeUntilNextUpdate > 0.0f)
        {
            continue;
        }

        State.TimeUntilNextUpdate = UpdateInterval;
    }
}
