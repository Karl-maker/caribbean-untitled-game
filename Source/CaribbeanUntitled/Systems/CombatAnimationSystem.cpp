//
// CombatAnimationSystem.cpp
// CaribbeanUntitled
//

#include "CombatAnimationSystem.h"

#include "AnimationSystem.h"
#include "../Components/AnimationRegistryComponent.h"
#include "../Components/CombatComponent.h"
#include "../EventBus/EventBusSubsystem.h"
#include "../Systems/SetupAttackSystem.h"

#include "Engine/World.h"

void UCombatAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);


    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Subscribe<FAttackStartedPayload>(this, &UCombatAnimationSystem::OnAttackStarted);
    }
    else
    {
    }
}

void UCombatAnimationSystem::Deinitialize()
{
    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Unsubscribe(this);
    }

    Super::Deinitialize();
}

UAnimationSystem* UCombatAnimationSystem::GetAnimationSystem() const
{
    UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UAnimationSystem>() : nullptr;
}

UEventBusSubsystem* UCombatAnimationSystem::GetEventBus() const
{
    UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UEventBusSubsystem>() : nullptr;
}

UAnimationRegistryComponent* UCombatAnimationSystem::FindAnimationRegistry(AActor* Actor) const
{
    return Actor ? Actor->FindComponentByClass<UAnimationRegistryComponent>() : nullptr;
}

void UCombatAnimationSystem::OnAttackStarted(const FAttackStartedPayload& Payload)
{
    if (!IsValid(Payload.SourceActor))
    {
        return;
    }

    UCombatComponent* Combat = Payload.SourceActor->FindComponentByClass<UCombatComponent>();
    if (!IsValid(Combat))
    {
        return;
    }

    const EAnimationKey AnimationKey = Combat->GetCurrentAttackAnimationKey();
    if (AnimationKey == EAnimationKey::None)
    {
        return;
    }

    EAnimationLayer AnimationLayer = EAnimationLayer::UpperBody;

    if (UAnimationRegistryComponent* Registry = FindAnimationRegistry(Payload.SourceActor))
    {
        if (const FAnimationDefinition* Definition = Registry->FindAnimation(AnimationKey))
        {
            AnimationLayer = Definition->Layer;
        }
    }


    if (UAnimationSystem* AnimationSystem = GetAnimationSystem())
    {
        AnimationSystem->RequestAnimationState(
            Payload.SourceActor,
            AnimationKey,
            true,
            AnimationLayer
        );
    }
    else
    {
    }
}
