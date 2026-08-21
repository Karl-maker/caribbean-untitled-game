//
//  TimeAttackSystem.cpp
//  CaribbeanUntitled
//

#include "TimeAttackSystem.h"

#include "../Components/AttackStateComponent.h"
#include "../Components/CombatComponent.h"
#include "../EventBus/EventBusSubsystem.h"
#include "ActorRegistry.h"
#include "AnimationSystem.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
float GetPhaseDuration(const FGameplayCombatMove& Move, EAttackPhase Phase)
{
    switch (Phase)
    {
        case EAttackPhase::Anticipation:
            return Move.AnticipationTime;
        case EAttackPhase::Active:
            return Move.ActiveTime;
        case EAttackPhase::Recovery:
            return Move.RecoveryTime;
        default:
            return 0.0f;
    }
}
}

void UTimeAttackSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    SubscribeToEvents();
}

void UTimeAttackSystem::Deinitialize()
{
    UnsubscribeFromEvents();
    ActiveAttacks.Empty();
    Super::Deinitialize();
}

TStatId UTimeAttackSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UTimeAttackSystem, STATGROUP_Tickables);
}

void UTimeAttackSystem::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.0f || ActiveAttacks.Num() == 0)
    {
        return;
    }

    UActorRegistry* Registry = GetActorRegistry();
    if (!Registry)
    {
        return;
    }

    for (auto It = ActiveAttacks.CreateIterator(); It; ++It)
    {
        AActor* Actor = It->Key.Get();
        if (!IsValid(Actor))
        {
            It.RemoveCurrent();
            continue;
        }

        ProcessActor(Actor, It->Value, DeltaTime);
    }
}

UEventBusSubsystem* UTimeAttackSystem::GetEventBus() const
{
    UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UEventBusSubsystem>() : nullptr;
}

UActorRegistry* UTimeAttackSystem::GetActorRegistry() const
{
    UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UActorRegistry>() : nullptr;
}

void UTimeAttackSystem::SubscribeToEvents()
{
    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Subscribe<FAttackStartedPayload>(this, &UTimeAttackSystem::OnAttackStarted);
    }
}

void UTimeAttackSystem::UnsubscribeFromEvents()
{
    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Unsubscribe(this);
    }
}

void UTimeAttackSystem::OnAttackStarted(const FAttackStartedPayload& Payload)
{
    if (!IsValid(Payload.SourceActor))
    {
        return;
    }

    FActiveAttackState& State = ActiveAttacks.FindOrAdd(Payload.SourceActor);
    State.Move = Payload.Move;
    State.bCombatVolumeActive = false;
    State.bRecoveryComplete = false;

    if (UAnimationSystem* AnimationSystem = GetWorld() ? GetWorld()->GetSubsystem<UAnimationSystem>() : nullptr)
    {
        AnimationSystem->RequestAnimationState(
            Payload.SourceActor,
            Payload.Move.Animation
        );
    }
}

void UTimeAttackSystem::SetMoveVolumesActive(const FGameplayCombatMove& Move, bool bActive) const
{
    for (TObjectPtr<UCombatVolumeComponent> Volume : Move.CombatVolumes)
    {
        if (IsValid(Volume))
        {
            Volume->SetAttackActive(bActive);
        }
    }
}

void UTimeAttackSystem::ProcessActor(AActor* Actor, FActiveAttackState& State, float DeltaTime)
{
    UAttackStateComponent* AttackState = Actor->FindComponentByClass<UAttackStateComponent>();
    UCombatComponent* Combat = Actor->FindComponentByClass<UCombatComponent>();

    if (!IsValid(AttackState) || !IsValid(Combat))
    {
        ActiveAttacks.Remove(Actor);
        return;
    }

    const EAttackPhase Phase = AttackState->GetPhase();
    const float PhaseTime = AttackState->GetPhaseTime();
    const float PhaseDuration = GetPhaseDuration(State.Move, Phase);

    if (Phase == EAttackPhase::Anticipation)
    {
        if (PhaseDuration > 0.0f && PhaseTime >= PhaseDuration)
        {
            AttackState->StartActivePhase();
            return;
        }
    }
    else if (Phase == EAttackPhase::Active)
    {
        if (!State.bCombatVolumeActive)
        {
            SetMoveVolumesActive(State.Move, true);
            State.bCombatVolumeActive = true;
        }

        if (PhaseDuration > 0.0f && PhaseTime >= PhaseDuration)
        {
            SetMoveVolumesActive(State.Move, false);
            State.bCombatVolumeActive = false;
            AttackState->StartRecoveryPhase();
            return;
        }
    }
    else if (Phase == EAttackPhase::Recovery)
    {
        if (State.bCombatVolumeActive)
        {
            SetMoveVolumesActive(State.Move, false);
            State.bCombatVolumeActive = false;
        }

        if (PhaseDuration > 0.0f && PhaseTime >= PhaseDuration)
        {
            Combat->SetCanAttack(true);
            AttackState->ClearAttackState();
            ActiveAttacks.Remove(Actor);
            return;
        }
    }
    else if (Phase == EAttackPhase::None)
    {
        if (State.bCombatVolumeActive)
        {
            SetMoveVolumesActive(State.Move, false);
        }

        ActiveAttacks.Remove(Actor);
    }
}
