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
    Tick(GetWorld(), DeltaTime);
}

void USetupAttackSystem::Tick(UWorld* World, float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    if (!World)
    {
        return;
    }

    UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();
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

void USetupAttackSystem::RequestAttackForActor(
    AActor* Actor,
    ECombatAction RequestedAction)
{
    if (!IsValid(Actor))
    {
        return;
    }

    UCombatComponent* Combat = Actor->FindComponentByClass<UCombatComponent>();
    UMoveSetComponent* MoveSet = Actor->FindComponentByClass<UMoveSetComponent>();
    UAttackStateComponent* AttackState = Actor->FindComponentByClass<UAttackStateComponent>();

    if (!IsValid(Combat) || !IsValid(MoveSet) || !IsValid(AttackState))
    {
        return;
    }

    StartAttack(
        Actor,
        Combat,
        MoveSet,
        AttackState,
        RequestedAction
    );
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

    StartAttack(
        Actor,
        Combat,
        MoveSet,
        AttackState,
        RequestedAction
    );
}

void USetupAttackSystem::StartAttack(
    AActor* Actor,
    UCombatComponent* Combat,
    UMoveSetComponent* MoveSet,
    UAttackStateComponent* AttackState,
    ECombatAction RequestedAction)
{
    if (!IsValid(Actor) || !IsValid(Combat) || !IsValid(MoveSet) || !IsValid(AttackState))
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
        return;
    }


    Combat->SetCanAttack(false);
    Combat->ClearRequestedAction();
    Combat->SetCurrentAttackAnimationKey(Move.Animation);
    Combat->TriggerAttackAnimationReplication();
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
    else
    {
    }
}
