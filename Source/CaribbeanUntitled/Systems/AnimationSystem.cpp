//
// AnimationSystem.cpp
// CaribbeanUntitled
//

#include "AnimationSystem.h"

#include "../Components/AnimationRegistryComponent.h"
#include "../EventBus/EventBusSubsystem.h"
#include "ActorRegistry.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"


//=============================================================================
// USubsystem
//=============================================================================

void UAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    BuildResolvedRules();

    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Subscribe<FAnimationRequestPayload>(
            this,
            &UAnimationSystem::OnAnimationRequest
        );
    }
}

void UAnimationSystem::Deinitialize()
{
    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Unsubscribe(this);
    }

    ActorStates.Empty();

    Super::Deinitialize();
}


//=============================================================================
// Requests
//=============================================================================

void UAnimationSystem::RequestAnimationState(
    AActor* Actor,
    EAnimationKey NewState,
    bool bForce
)
{
    if (!Actor || NewState == EAnimationKey::None)
    {
        return;
    }

    FActorAnimationState& State = GetOrCreateActorState(Actor);

    const bool bAlreadyThere =
        !bForce &&
        NewState == State.CurrentState &&
        State.PendingQueue.Num() == 0;

    if (bAlreadyThere)
    {
        // Same state re-requested (e.g. a system re-broadcasting
        // "Sprint" every tick) — avoid restarting the animation.
        return;
    }

    const UWorld* World = GetWorld();
    const double Now = World ? World->GetTimeSeconds() : 0.0;
    const double TimeInFromState = Now - State.StateEnteredTime;

    const FActorAnimTransitionRule* Rule = bForce
        ? nullptr
        : FindTransitionRule(State.CurrentState, NewState, TimeInFromState);

    EAnimationKey ResolvedTarget = NewState;
    TArray<EAnimationKey> Chain;

    if (Rule)
    {
        if (Rule->OverrideResultState != EAnimationKey::None)
        {
            ResolvedTarget = Rule->OverrideResultState;
        }

        Chain = Rule->IntermediateStates;
    }

    Chain.Add(ResolvedTarget);

    State.PendingQueue = Chain;

    AdvanceQueue(Actor, State);
}

EAnimationKey UAnimationSystem::GetCurrentState(AActor* Actor) const
{
    if (const FActorAnimationState* State = ActorStates.Find(Actor))
    {
        return State->CurrentState;
    }

    return EAnimationKey::None;
}

void UAnimationSystem::ClearActorState(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    if (FActorAnimationState* State = ActorStates.Find(Actor))
    {
        if (UAnimMontage* Montage = State->ActiveMontage.Get())
        {
            if (USkeletalMeshComponent* Mesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
            {
                if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
                {
                    AnimInstance->Montage_Stop(Montage->BlendOut.GetBlendTime(), Montage);
                }
            }
        }
    }

    ActorStates.Remove(Actor);
}


//=============================================================================
// Per-actor state
//=============================================================================

UAnimationSystem::FActorAnimationState&
UAnimationSystem::GetOrCreateActorState(AActor* Actor)
{
    if (FActorAnimationState* Existing = ActorStates.Find(Actor))
    {
        return *Existing;
    }

    PruneStaleActors();

    return ActorStates.Add(Actor, FActorAnimationState());
}

void UAnimationSystem::PruneStaleActors()
{
    for (auto It = ActorStates.CreateIterator(); It; ++It)
    {
        if (!It->Key.IsValid())
        {
            It.RemoveCurrent();
        }
    }
}


//=============================================================================
// Transition resolution
//=============================================================================

void UAnimationSystem::BuildResolvedRules()
{
    ResolvedRules.Reset();

    if (TransitionRulesTable)
    {
        TArray<FActorAnimTransitionRule*> Rows;
        TransitionRulesTable->GetAllRows<FActorAnimTransitionRule>(
            TEXT("UAnimationSystem::BuildResolvedRules"),
            Rows
        );

        for (const FActorAnimTransitionRule* Row : Rows)
        {
            if (Row)
            {
                ResolvedRules.Add(*Row);
            }
        }
    }

    ResolvedRules.Append(TransitionRules);
}

const FActorAnimTransitionRule* UAnimationSystem::FindTransitionRule(
    EAnimationKey FromState,
    EAnimationKey ToState,
    double TimeInFromState
) const
{
    const FActorAnimTransitionRule* WildcardMatch = nullptr;

    for (const FActorAnimTransitionRule& Rule : ResolvedRules)
    {
        if (Rule.ToState != ToState)
        {
            continue;
        }

        const bool bIsComboRule = Rule.OverrideResultState != EAnimationKey::None;

        if (bIsComboRule &&
            Rule.ComboWindowSeconds > 0.0f &&
            TimeInFromState > static_cast<double>(Rule.ComboWindowSeconds))
        {
            // Window expired — this rule no longer applies.
            continue;
        }

        if (Rule.FromState == FromState)
        {
            // Exact match wins immediately.
            return &Rule;
        }

        if (Rule.FromState == EAnimationKey::None && !WildcardMatch)
        {
            WildcardMatch = &Rule;
        }
    }

    return WildcardMatch;
}


//=============================================================================
// Playback
//=============================================================================

void UAnimationSystem::AdvanceQueue(AActor* Actor, FActorAnimationState& State)
{
    if (State.PendingQueue.Num() == 0)
    {
        return;
    }

    const EAnimationKey NextKey = State.PendingQueue[0];
    State.PendingQueue.RemoveAt(0);

    UAnimMontage* Montage = PlayStateAnimation(Actor, NextKey);

    State.CurrentState = NextKey;
    State.ActiveMontage = Montage;

    const UWorld* World = GetWorld();
    State.StateEnteredTime = World ? World->GetTimeSeconds() : 0.0;

    if (!Montage)
    {
        // Nothing to play (missing definition/asset, or no AnimInstance
        // to drive) — don't stall the chain, move straight to the next
        // queued entry, if any.
        if (State.PendingQueue.Num() > 0)
        {
            AdvanceQueue(Actor, State);
        }

        return;
    }

    if (USkeletalMeshComponent* Mesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
        {
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(
                this,
                &UAnimationSystem::OnStateAnimationEnded,
                TWeakObjectPtr<AActor>(Actor)
            );
            AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
        }
    }
}

UAnimMontage* UAnimationSystem::PlayStateAnimation(AActor* Actor, EAnimationKey Key)
{
    UAnimationRegistryComponent* Registry = FindAnimationRegistry(Actor);
    if (!Registry)
    {
        return nullptr;
    }

    FAnimationDefinition Definition;
    if (!Registry->TryFindAnimation(Key, Definition) || !Definition.Animation)
    {
        return nullptr;
    }

    USkeletalMeshComponent* Mesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh)
    {
        return nullptr;
    }

    UAnimInstance* AnimInstance = Mesh->GetAnimInstance();

    // Preferred path: driven through an AnimBP via a dynamic montage on
    // AnimationSlotName. This is what makes BlendInTime/BlendOutTime,
    // PlayRate, StartPosition and LoopCount (all authored per-key on
    // FAnimationDefinition) actually take effect, and gives us the
    // OnMontageEnded hook the state machine chains off of.
    if (AnimInstance)
    {
        if (UAnimSequenceBase* SequenceBase = Cast<UAnimSequenceBase>(Definition.Animation))
        {
            const int32 LoopCount =
                Definition.PlaybackMode == EAnimationPlaybackMode::Loop
                    ? INDEX_NONE // PlaySlotAnimationAsDynamicMontage treats <0 as infinite looping.
                    : FMath::Max(1, Definition.LoopCount);

            return AnimInstance->PlaySlotAnimationAsDynamicMontage(
                SequenceBase,
                AnimationSlotName,
                Definition.BlendInTime,
                Definition.BlendOutTime,
                Definition.PlayRate,
                LoopCount,
                /*BlendOutTriggerTime=*/ -1.0f,
                Definition.StartPosition
            );
        }
    }

    // Fallback for meshes with no AnimBP: play the asset directly.
    // No chaining hook is available here, so a Hold/Loop definition
    // will simply keep playing until superseded by the next request.
    const bool bLooping = Definition.PlaybackMode == EAnimationPlaybackMode::Loop;
    Mesh->PlayAnimation(Definition.Animation, bLooping);

    return nullptr;
}

void UAnimationSystem::OnStateAnimationEnded(
    UAnimMontage* Montage,
    bool bInterrupted,
    TWeakObjectPtr<AActor> Actor
)
{
    if (bInterrupted || !Actor.IsValid())
    {
        return;
    }

    FActorAnimationState* State = ActorStates.Find(Actor);
    if (!State || State->ActiveMontage.Get() != Montage)
    {
        // A newer request already replaced this montage; ignore the
        // stale callback.
        return;
    }

    if (State->PendingQueue.Num() > 0)
    {
        AdvanceQueue(Actor.Get(), *State);
    }
}


//=============================================================================
// Lookups
//=============================================================================

UAnimationRegistryComponent* UAnimationSystem::FindAnimationRegistry(AActor* Actor) const
{
    if (!Actor)
    {
        return nullptr;
    }

    return Actor->FindComponentByClass<UAnimationRegistryComponent>();
}

UEventBusSubsystem* UAnimationSystem::GetEventBus() const
{
    const UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UEventBusSubsystem>() : nullptr;
}

UActorRegistry* UAnimationSystem::GetActorRegistry() const
{
    const UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UActorRegistry>() : nullptr;
}

void UAnimationSystem::OnAnimationRequest(const FAnimationRequestPayload& Payload)
{
    RequestAnimationState(Payload.Actor, Payload.RequestedState, Payload.bForce);
}
