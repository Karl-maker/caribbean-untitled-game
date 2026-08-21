//
//  SetupAttackSystem.cpp
//  CaribbeanUntitled
//

#include "SetupAttackSystem.h"

#include "../Components/AttackStateComponent.h"
#include "../Components/CombatComponent.h"
#include "../Components/MoveSetComponent.h"
#include "../EventBus/EventBusSubsystem.h"
#include "ActorRegistry.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

void USetupAttackSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void USetupAttackSystem::Deinitialize()
{
    Super::Deinitialize();
}

TStatId USetupAttackSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USetupAttackSystem, STATGROUP_Tickables);
}

void USetupAttackSystem::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    UActorRegistry* Registry = GetActorRegistry();
    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors =
        Registry->GetActorsWithComponent<UCombatComponent>();

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

        ProcessActor(Actor);
    }
}

UActorRegistry* USetupAttackSystem::GetActorRegistry() const
{
    UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UActorRegistry>() : nullptr;
}

UEventBusSubsystem* USetupAttackSystem::GetEventBus() const
{
    UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UEventBusSubsystem>() : nullptr;
}

bool USetupAttackSystem::ResolveMoveKey(ECombatAction Action, EMoveSetTypeKey& OutMoveKey) const
{
    switch (Action)
    {
        case ECombatAction::LightAttack:
            OutMoveKey = EMoveSetTypeKey::LightAttack1;
            return true;

        case ECombatAction::HeavyAttack:
            OutMoveKey = EMoveSetTypeKey::HeavyAttack1;
            return true;

        default:
            OutMoveKey = EMoveSetTypeKey::None;
            return false;
    }
}

void USetupAttackSystem::ProcessActor(AActor* Actor)
{
    UCombatComponent* Combat = Actor->FindComponentByClass<UCombatComponent>();
    UMoveSetComponent* MoveSet = Actor->FindComponentByClass<UMoveSetComponent>();
    UAttackStateComponent* AttackState = Actor->FindComponentByClass<UAttackStateComponent>();

    if (!IsValid(Combat) || !IsValid(MoveSet) || !IsValid(AttackState))
    {
        return;
    }

    const ECombatAction RequestedAction = Combat->GetRequestedAction();
    if (RequestedAction == ECombatAction::None)
    {
        return;
    }

    EMoveSetTypeKey MoveKey = EMoveSetTypeKey::None;
    if (!ResolveMoveKey(RequestedAction, MoveKey))
    {
        return;
    }

    const FGameplayCombatMove Move = MoveSet->GetMove(MoveKey);
    if (Move.MoveId.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("SetupAttackSystem: No move found for key %d on %s"), static_cast<int32>(MoveKey), *Actor->GetName());
        return;
    }

    Combat->SetCanAttack(false);
    AttackState->SetPhaseTime(0.0f);
    AttackState->StartAnticipationWithMove(static_cast<int32>(MoveKey), MoveKey, Move.MoveId);

    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        FAttackStartedPayload Payload;
        Payload.SourceActor = Actor;
        Payload.CombatAction = RequestedAction;
        Payload.MoveKey = MoveKey;
        Payload.Move = Move;
        EventBus->Publish<FAttackStartedPayload>(Payload);
    }
}
