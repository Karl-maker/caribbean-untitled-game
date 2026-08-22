//
// CombatAnimationSystem.cpp
// CaribbeanUntitled
//

#include "CombatAnimationSystem.h"

#include "AnimationSystem.h"
#include "../Components/CombatComponent.h"
#include "../EventBus/EventBusSubsystem.h"
#include "../Systems/SetupAttackSystem.h"

#include "Engine/World.h"

void UCombatAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Log, TEXT("CombatAnimationSystem: Initializing"));

    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Subscribe<FAttackStartedPayload>(this, &UCombatAnimationSystem::OnAttackStarted);
        UE_LOG(LogTemp, Log, TEXT("CombatAnimationSystem: Subscribed to FAttackStartedPayload"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatAnimationSystem: EventBus unavailable during Initialize"));
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

void UCombatAnimationSystem::OnAttackStarted(const FAttackStartedPayload& Payload)
{
    if (!IsValid(Payload.SourceActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatAnimationSystem: Invalid SourceActor in FAttackStartedPayload"));
        return;
    }

    UCombatComponent* Combat = Payload.SourceActor->FindComponentByClass<UCombatComponent>();
    if (!IsValid(Combat))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("CombatAnimationSystem: No CombatComponent on %s"),
            *Payload.SourceActor->GetName()
        );
        return;
    }

    const EAnimationKey AnimationKey = Combat->GetCurrentAttackAnimationKey();
    if (AnimationKey == EAnimationKey::None)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("CombatAnimationSystem: No attack animation key set on %s"),
            *Payload.SourceActor->GetName()
        );
        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("CombatAnimationSystem: Requesting animation %d for %s with force restart"),
        static_cast<int32>(AnimationKey),
        *Payload.SourceActor->GetName()
    );

    if (UAnimationSystem* AnimationSystem = GetAnimationSystem())
    {
        AnimationSystem->RequestAnimationState(
            Payload.SourceActor,
            AnimationKey,
            true
        );
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatAnimationSystem: AnimationSystem unavailable"));
    }
}
