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
    bool bForce,
    EAnimationLayer Layer
)
{
    if (!Actor || NewState == EAnimationKey::None)
    {
        return;
    }

    // Every layer has its own independent animation state.
    FActorAnimationState& State =
        GetOrCreateActorState(Actor, Layer);

    const bool bAlreadyThere =
        !bForce &&
        NewState == State.CurrentState &&
        State.PendingQueue.Num() == 0;

    if (bAlreadyThere)
    {
        // Same state re-requested.
        //
        // Example:
        // Movement system broadcasts Sprint every tick.
        //
        // Don't restart the animation.
        return;
    }

    const UWorld* World = GetWorld();

    const double Now =
        World ? World->GetTimeSeconds() : 0.0;

    const double TimeInFromState =
        Now - State.StateEnteredTime;

    const FActorAnimTransitionRule* Rule =
        bForce
            ? nullptr
            : FindTransitionRule(
                State.CurrentState,
                NewState,
                TimeInFromState
            );

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

    AdvanceQueue(
        Actor,
        State,
        Layer
    );
}


//=============================================================================
// Current State
//=============================================================================

EAnimationKey UAnimationSystem::GetCurrentState(
    AActor* Actor
) const
{
    if (!Actor)
    {
        return EAnimationKey::None;
    }

    // Backwards-compatible behavior:
    // GetCurrentState() means Base layer.
    const FActorAnimationState* State =
        const_cast<UAnimationSystem*>(this)
            ->FindActorAnimationState(
                Actor,
                EAnimationLayer::Base
            );

    if (State)
    {
        return State->CurrentState;
    }

    return EAnimationKey::None;
}


//=============================================================================
// Clear Actor
//=============================================================================

void UAnimationSystem::ClearActorState(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    FActorAnimationLayers* ActorLayers =
        ActorStates.Find(Actor);

    if (!ActorLayers)
    {
        return;
    }

    USkeletalMeshComponent* Mesh =
        Actor->FindComponentByClass<USkeletalMeshComponent>();

    UAnimInstance* AnimInstance =
        Mesh ? Mesh->GetAnimInstance() : nullptr;

    if (AnimInstance)
    {
        // Stop every active montage belonging to every layer.
        for (auto& LayerPair : ActorLayers->Layers)
        {
            FActorAnimationState& State =
                LayerPair.Value;

            if (UAnimMontage* Montage =
                State.ActiveMontage.Get())
            {
                AnimInstance->Montage_Stop(
                    Montage->BlendOut.GetBlendTime(),
                    Montage
                );
            }
        }
    }

    ActorStates.Remove(Actor);
}


//=============================================================================
// Per-actor / Per-layer state
//=============================================================================

UAnimationSystem::FActorAnimationState&
UAnimationSystem::GetOrCreateActorState(
    AActor* Actor,
    EAnimationLayer Layer
)
{
    if (!Actor)
    {
        // This should never happen because callers validate Actor.
        // Returning a static state avoids undefined behavior.
        static FActorAnimationState InvalidState;
        return InvalidState;
    }

    FActorAnimationLayers* ActorLayers =
        ActorStates.Find(Actor);

    if (!ActorLayers)
    {
        PruneStaleActors();

        ActorLayers = &ActorStates.Add(
            Actor,
            FActorAnimationLayers()
        );
    }

    // Create the state for this layer if it doesn't exist.
    return ActorLayers->Layers.FindOrAdd(
        Layer,
        FActorAnimationState()
    );
}


UAnimationSystem::FActorAnimationState*
UAnimationSystem::FindActorAnimationState(
    AActor* Actor,
    EAnimationLayer Layer
)
{
    if (!Actor)
    {
        return nullptr;
    }

    FActorAnimationLayers* ActorLayers =
        ActorStates.Find(Actor);

    if (!ActorLayers)
    {
        return nullptr;
    }

    return ActorLayers->Layers.Find(Layer);
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


const FActorAnimTransitionRule*
UAnimationSystem::FindTransitionRule(
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

        const bool bIsComboRule =
            Rule.OverrideResultState != EAnimationKey::None;

        if (
            bIsComboRule &&
            Rule.ComboWindowSeconds > 0.0f &&
            TimeInFromState >
                static_cast<double>(Rule.ComboWindowSeconds)
        )
        {
            continue;
        }

        if (Rule.FromState == FromState)
        {
            // Exact match wins.
            return &Rule;
        }

        if (
            Rule.FromState == EAnimationKey::None &&
            !WildcardMatch
        )
        {
            WildcardMatch = &Rule;
        }
    }

    return WildcardMatch;
}


//=============================================================================
// Playback
//=============================================================================

void UAnimationSystem::AdvanceQueue(
    AActor* Actor,
    FActorAnimationState& State,
    EAnimationLayer Layer
)
{
    if (!Actor)
    {
        return;
    }

    if (State.PendingQueue.Num() == 0)
    {
        return;
    }

    const EAnimationKey NextKey =
        State.PendingQueue[0];

    State.PendingQueue.RemoveAt(0);

    UAnimMontage* Montage =
        PlayStateAnimation(
            Actor,
            NextKey
        );

    State.CurrentState = NextKey;
    State.ActiveMontage = Montage;

    const UWorld* World = GetWorld();

    State.StateEnteredTime =
        World ? World->GetTimeSeconds() : 0.0;

    if (!Montage)
    {
        // Missing animation / AnimInstance / asset.
        //
        // Don't stall a transition chain.
        if (State.PendingQueue.Num() > 0)
        {
            AdvanceQueue(
                Actor,
                State,
                Layer
            );
        }

        return;
    }

    USkeletalMeshComponent* Mesh =
        Actor->FindComponentByClass<USkeletalMeshComponent>();

    if (!Mesh)
    {
        return;
    }

    UAnimInstance* AnimInstance =
        Mesh->GetAnimInstance();

    if (!AnimInstance)
    {
        return;
    }

    FOnMontageEnded EndDelegate;

    EndDelegate.BindUObject(
        this,
        &UAnimationSystem::OnStateAnimationEnded,
        TWeakObjectPtr<AActor>(Actor),
        Layer
    );

    AnimInstance->Montage_SetEndDelegate(
        EndDelegate,
        Montage
    );
}


//=============================================================================
// Play Animation
//=============================================================================

UAnimMontage*
UAnimationSystem::PlayStateAnimation(
    AActor* Actor,
    EAnimationKey Key
)
{
    if (!Actor)
    {
        return nullptr;
    }

    UAnimationRegistryComponent* Registry =
        FindAnimationRegistry(Actor);

    if (!Registry)
    {
        return nullptr;
    }

    FAnimationDefinition Definition;

    if (
        !Registry->TryFindAnimation(
            Key,
            Definition
        ) ||
        !Definition.Animation
    )
    {
        return nullptr;
    }

    USkeletalMeshComponent* Mesh =
        Actor->FindComponentByClass<USkeletalMeshComponent>();

    if (!Mesh)
    {
        return nullptr;
    }

    UAnimInstance* AnimInstance =
        Mesh->GetAnimInstance();

    //=========================================================================
    // AnimBP / Dynamic Montage
    //=========================================================================

    if (AnimInstance)
    {
        if (
            UAnimSequenceBase* SequenceBase =
                Cast<UAnimSequenceBase>(
                    Definition.Animation
                )
        )
        {
            const int32 LoopCount =
                Definition.PlaybackMode ==
                    EAnimationPlaybackMode::Loop

                    ? INDEX_NONE

                    : FMath::Max(
                        1,
                        Definition.LoopCount
                    );

            return AnimInstance
                ->PlaySlotAnimationAsDynamicMontage(
                    SequenceBase,

                    // Current setup still uses the subsystem's slot.
                    Definition.SlotName,

                    Definition.BlendInTime,
                    Definition.BlendOutTime,
                    Definition.PlayRate,
                    LoopCount,

                    /* BlendOutTriggerTime */
                    -1.0f,

                    Definition.StartPosition
                );
        }
    }

    //=========================================================================
    // Fallback
    //=========================================================================

    const bool bLooping =
        Definition.PlaybackMode ==
        EAnimationPlaybackMode::Loop;

    Mesh->PlayAnimation(
        Definition.Animation,
        bLooping
    );

    return nullptr;
}


//=============================================================================
// Montage End
//=============================================================================

void UAnimationSystem::OnStateAnimationEnded(
    UAnimMontage* Montage,
    bool bInterrupted,
    TWeakObjectPtr<AActor> Actor,
    EAnimationLayer Layer
)
{
    if (
        bInterrupted ||
        !Actor.IsValid() ||
        !Montage
    )
    {
        return;
    }

    FActorAnimationState* State =
        FindActorAnimationState(
            Actor.Get(),
            Layer
        );

    if (!State)
    {
        return;
    }

    // A newer request may have already replaced
    // this montage on this specific layer.
    if (State->ActiveMontage.Get() != Montage)
    {
        return;
    }

    if (State->PendingQueue.Num() > 0)
    {
        AdvanceQueue(
            Actor.Get(),
            *State,
            Layer
        );
    }
}


//=============================================================================
// Lookups
//=============================================================================

UAnimationRegistryComponent*
UAnimationSystem::FindAnimationRegistry(
    AActor* Actor
) const
{
    if (!Actor)
    {
        return nullptr;
    }

    return Actor->FindComponentByClass<
        UAnimationRegistryComponent
    >();
}


UEventBusSubsystem*
UAnimationSystem::GetEventBus() const
{
    const UWorld* World = GetWorld();

    return World
        ? World->GetSubsystem<UEventBusSubsystem>()
        : nullptr;
}


UActorRegistry*
UAnimationSystem::GetActorRegistry() const
{
    const UWorld* World = GetWorld();

    return World
        ? World->GetSubsystem<UActorRegistry>()
        : nullptr;
}


//=============================================================================
// Event Bus
//=============================================================================

void UAnimationSystem::OnAnimationRequest(
    const FAnimationRequestPayload& Payload
)
{
    RequestAnimationState(
        Payload.Actor,
        Payload.RequestedState,
        Payload.bForce,
        Payload.Layer
    );
}
