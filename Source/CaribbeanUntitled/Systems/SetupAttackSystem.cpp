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

void USetupAttackSystem::RequestAttackForActor(
    AActor* Actor,
    ECombatAction RequestedAction)
{
    if (!IsValid(Actor))
    {
        UE_LOG(LogTemp, Warning, TEXT("SetupAttackSystem: RequestAttackForActor called with invalid actor"));
        return;
    }

    UCombatComponent* Combat = Actor->FindComponentByClass<UCombatComponent>();
    UMoveSetComponent* MoveSet = Actor->FindComponentByClass<UMoveSetComponent>();
    UAttackStateComponent* AttackState = Actor->FindComponentByClass<UAttackStateComponent>();

    if (!IsValid(Combat) || !IsValid(MoveSet) || !IsValid(AttackState))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("SetupAttackSystem: RequestAttackForActor missing combat/moveset/attackstate on %s"),
            *GetNameSafe(Actor)
        );
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
        UE_LOG(
            LogTemp,
            Verbose,
            TEXT("SetupAttackSystem: Missing combat/moveset/attackstate on %s"),
            *GetNameSafe(Actor)
        );
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
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("SetupAttackSystem: Could not resolve move key for %s action %d"),
            *Actor->GetName(),
            static_cast<int32>(RequestedAction)
        );
        return;
    }

    const FGameplayCombatMove Move = MoveSet->GetMove(MoveKey);
    if (Move.MoveId.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("SetupAttackSystem: No move found for key %d on %s"), static_cast<int32>(MoveKey), *Actor->GetName());
        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("SetupAttackSystem: Starting attack on %s | Action=%d MoveKey=%d AnimKey=%d MoveId=%s"),
        *Actor->GetName(),
        static_cast<int32>(RequestedAction),
        static_cast<int32>(MoveKey),
        static_cast<int32>(Move.Animation),
        *Move.MoveId.ToString()
    );

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
        UE_LOG(
            LogTemp,
            Log,
            TEXT("SetupAttackSystem: Publishing FAttackStartedPayload for %s"),
            *Actor->GetName()
        );
        EventBus->Publish<FAttackStartedPayload>(Payload);
    }
    else
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("SetupAttackSystem: No EventBus available when starting attack on %s"),
            *Actor->GetName()
        );
    }
}
