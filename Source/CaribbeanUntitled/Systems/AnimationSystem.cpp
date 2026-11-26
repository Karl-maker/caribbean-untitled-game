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

namespace
{
    FName ResolveSlotName(EAnimationLayer Layer)
    {
        switch (Layer)
        {
            case EAnimationLayer::UpperBody:
                return AnimationSlots::UpperBody;
            case EAnimationLayer::Face:
                return AnimationSlots::Face;
            case EAnimationLayer::IK:
                return AnimationSlots::IK;
            case EAnimationLayer::FullBody:
            case EAnimationLayer::Additive:
            default:
                return AnimationSlots::FullBody;
        }
    }
}


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
    else
    {
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

void UAnimationSystem::Tick(float DeltaTime)
{
    Tick(GetWorld(), DeltaTime);
}

void UAnimationSystem::Tick(UWorld* World, float DeltaTime)
{
    (void)World;
    (void)DeltaTime;
    PruneStaleActors();
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
    if (!Actor)
    {
        return;
    }

    if (NewState == EAnimationKey::None)
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
        // Don't restart the animation.
        return;
    }


    USkeletalMeshComponent* Mesh =
        Actor->FindComponentByClass<USkeletalMeshComponent>();

    UAnimInstance* AnimInstance =
        Mesh ? Mesh->GetAnimInstance() : nullptr;

    if (AnimInstance)
    {
        if (UAnimMontage* ActiveMontage = State.ActiveMontage.Get())
        {
            AnimInstance->Montage_Stop(
                FMath::Max(ActiveMontage->BlendOut.GetBlendTime(), 0.05f),
                ActiveMontage
            );
        }
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

    if (Rule && !bForce)
    {
    }
    else if (bForce)
    {
    }

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
    return GetCurrentState(Actor, EAnimationLayer::FullBody);
}

EAnimationKey UAnimationSystem::GetCurrentState(AActor* Actor, EAnimationLayer Layer) const
{
    if (!Actor)
    {
        return EAnimationKey::None;
    }

    const FActorAnimationState* State =
        const_cast<UAnimationSystem*>(this)->FindActorAnimationState(Actor, Layer);
    return State ? State->CurrentState : EAnimationKey::None;
}

float UAnimationSystem::GetStateElapsedTime(AActor* Actor, EAnimationLayer Layer) const
{
    if (!Actor)
    {
        return 0.0f;
    }

    const FActorAnimationState* State =
        const_cast<UAnimationSystem*>(this)->FindActorAnimationState(Actor, Layer);
    if (!State)
    {
        return 0.0f;
    }

    const UWorld* World = GetWorld();
    const double Now = World ? World->GetTimeSeconds() : 0.0;
    return static_cast<float>(FMath::Max(0.0, Now - State->StateEnteredTime));
}

const UAnimSequenceBase* UAnimationSystem::GetCurrentAnimation(AActor* Actor, EAnimationLayer Layer) const
{
    const FAnimationDefinition* Definition = GetCurrentAnimationDefinition(Actor, Layer);
    return Definition ? Cast<UAnimSequenceBase>(Definition->Animation) : nullptr;
}

const FAnimationDefinition* UAnimationSystem::GetCurrentAnimationDefinition(AActor* Actor, EAnimationLayer Layer) const
{
    if (!Actor)
    {
        return nullptr;
    }

    const FActorAnimationState* State =
        const_cast<UAnimationSystem*>(this)->FindActorAnimationState(Actor, Layer);
    if (!State)
    {
        return nullptr;
    }

    const UAnimationRegistryComponent* Registry = FindAnimationRegistry(Actor);
    return Registry ? Registry->FindAnimation(State->CurrentState) : nullptr;
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

    if (WildcardMatch)
    {
        return WildcardMatch;
    }


    return nullptr;
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


    UAnimMontage* Montage = nullptr;

    const bool bPlayed =
        PlayStateAnimation(
            Actor,
            NextKey,
            Layer,
            Montage
        );

    if (!bPlayed)
    {

        // Deliberately do NOT update State.CurrentState / ActiveMontage /
        // StateEnteredTime here. A genuine failure (missing anim, missing
        // AnimInstance during a startup race, etc.) must not "poison" this
        // state — a future request for the same NewState needs to retry
        // rather than being swallowed by RequestAnimationState's no-op
        // check, which compares against State.CurrentState.

        // Missing animation / AnimInstance / asset.
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

    // Success (either a real montage, or a fallback SkeletalMeshComponent
    // play) — this state is now genuinely current.
    State.CurrentState = NextKey;
    State.ActiveMontage = Montage; // may still be nullptr on the fallback path

    const UWorld* World = GetWorld();

    State.StateEnteredTime =
        World ? World->GetTimeSeconds() : 0.0;


    if (!Montage)
    {
        // Fallback playback path — no montage to bind an end delegate to.
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

bool UAnimationSystem::PlayStateAnimation(
    AActor* Actor,
    EAnimationKey Key,
    EAnimationLayer Layer,
    UAnimMontage*& OutMontage
)
{
    OutMontage = nullptr;

    if (!Actor)
    {
        return false;
    }


    UAnimationRegistryComponent* Registry =
        FindAnimationRegistry(Actor);

    if (!Registry)
    {
        return false;
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
        return false;
    }


    USkeletalMeshComponent* Mesh =
        Actor->FindComponentByClass<USkeletalMeshComponent>();

    if (!Mesh)
    {
        return false;
    }

    UAnimInstance* AnimInstance =
        Mesh->GetAnimInstance();

    //=========================================================================
    // AnimBP / Dynamic Montage
    //=========================================================================

    if (AnimInstance)
    {

        AnimInstance->SetRootMotionMode(
            ERootMotionMode::IgnoreRootMotion
        );

        if (
            UAnimSequenceBase* SequenceBase =
                Cast<UAnimSequenceBase>(
                    Definition.Animation
                )
        )
        {
            const int32 LoopCount =
                Definition.PlaybackMode == EAnimationPlaybackMode::Loop
                    ? EffectiveInfiniteLoopCount
                    : 1;

            OutMontage = AnimInstance
                ->PlaySlotAnimationAsDynamicMontage(
                    SequenceBase,

                    ResolveSlotName(Layer),

                    Definition.BlendInTime,
                    Definition.BlendOutTime,
                    Definition.PlayRate,
                    LoopCount,

                    /* BlendOutTriggerTime */
                    -1.0f,

                    Definition.StartPosition
                );

            if (!OutMontage)
            {
            }

            return OutMontage != nullptr;
        }
        else
        {
            return false;
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

    // OutMontage stays nullptr — PlayAnimation() produces no montage to
    // track, but playback itself genuinely succeeded.
    return true;
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
        if (bInterrupted)
        {
        }
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
    else
    {
        if (Layer != EAnimationLayer::FullBody)
        {
            State->CurrentState = EAnimationKey::None;
            State->ActiveMontage.Reset();
            State->StateEnteredTime = 0.0;
        }
        else
        {
        }
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

    return Actor->FindComponentByClass<UAnimationRegistryComponent>();
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
    // Check if the actor is valid using Get() and IsValid()
    AActor* Actor = Payload.Actor.Get();
    if (!Actor || !IsValid(Actor))
    {
        return;
    }


    RequestAnimationState(
        Actor,
        Payload.RequestedState,
        Payload.bForce,
        Payload.Layer
    );
}
