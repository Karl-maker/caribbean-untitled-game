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
#include "../Actors/CharacterActor.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

namespace
{
    FName ResolveSlotName(EAnimationSlot Slot)
    {
        switch (Slot)
        {
            case EAnimationSlot::UpperBody:
                return AnimationSlots::UpperBody;
            case EAnimationSlot::Face:
                return AnimationSlots::Face;
            case EAnimationSlot::IK:
                return AnimationSlots::IK;
            case EAnimationSlot::FullBody:
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

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Initializing"));

    BuildResolvedRules();

    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Subscribe<FAnimationRequestPayload>(
            this,
            &UAnimationSystem::OnAnimationRequest
        );
        UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Subscribed to FAnimationRequestPayload"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem: Failed to get EventBusSubsystem"));
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
        Registry->GetActorsWithComponent<UAnimationRegistryComponent>();
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

        if (ACharacterActor* CharacterActor = Cast<ACharacterActor>(Actor))
        {
            CharacterActor->SyncPoseableMeshFromSkeletalMesh();
        }
    }
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
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem::RequestAnimationState - Actor is null"));
        return;
    }

    if (NewState == EAnimationKey::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystem::RequestAnimationState - NewState is None for %s"),
               *Actor->GetName());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem::RequestAnimationState - Actor: %s, NewState: %d (%s), bForce: %d, Layer: %d"),
           *Actor->GetName(),
           (int32)NewState,
           *UEnum::GetValueAsString(NewState),
           bForce,
           (int32)Layer);

    // Every layer has its own independent animation state.
    FActorAnimationState& State =
        GetOrCreateActorState(Actor, Layer);

    UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem: Current state for %s is %d (%s)"),
           *Actor->GetName(),
           (int32)State.CurrentState,
           *UEnum::GetValueAsString(State.CurrentState));

    const bool bAlreadyThere =
        !bForce &&
        NewState == State.CurrentState &&
        State.PendingQueue.Num() == 0;

    if (bAlreadyThere)
    {
        // Same state re-requested.
        // Don't restart the animation.
        UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem: Skipping duplicate state %d for %s"),
               (int32)NewState, *Actor->GetName());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Processing state change from %d to %d for %s (bForce=%d)"),
           (int32)State.CurrentState, (int32)NewState, *Actor->GetName(), bForce);

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
        UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Found transition rule for %s from %d to %d"),
               *Actor->GetName(), (int32)State.CurrentState, (int32)NewState);
    }
    else if (bForce)
    {
        UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Bypassing transition rules due to bForce=true for %s"),
               *Actor->GetName());
    }

    EAnimationKey ResolvedTarget = NewState;

    TArray<EAnimationKey> Chain;

    if (Rule)
    {
        if (Rule->OverrideResultState != EAnimationKey::None)
        {
            ResolvedTarget = Rule->OverrideResultState;
            UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Override result state to %d for %s"),
                   (int32)ResolvedTarget, *Actor->GetName());
        }

        Chain = Rule->IntermediateStates;
        UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Added %d intermediate states"), Chain.Num());
    }

    Chain.Add(ResolvedTarget);

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Final chain has %d states for %s"),
           Chain.Num(), *Actor->GetName());

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
                EAnimationLayer::FullBody
            );

    if (State)
    {
        return State->CurrentState;
    }

    return EAnimationKey::None;
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
    if (!Registry)
    {
        return nullptr;
    }

    const FAnimationDefinition* Definition = Registry->FindAnimation(State->CurrentState);
    return Definition ? Cast<UAnimSequenceBase>(Definition->Animation) : nullptr;
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
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem::GetOrCreateActorState - Actor is null"));
        return InvalidState;
    }

    FActorAnimationLayers* ActorLayers =
        ActorStates.Find(Actor);

    if (!ActorLayers)
    {
        UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem: Creating new state for actor %s"), *Actor->GetName());
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

        UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Found %d transition rules in DataTable"), Rows.Num());

        for (const FActorAnimTransitionRule* Row : Rows)
        {
            if (Row)
            {
                ResolvedRules.Add(*Row);
            }
        }
    }

    ResolvedRules.Append(TransitionRules);

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Total %d transition rules"), ResolvedRules.Num());
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
            UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem: Combo window expired for rule from %d to %d"),
                   (int32)FromState, (int32)ToState);
            continue;
        }

        if (Rule.FromState == FromState)
        {
            // Exact match wins.
            UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem: Found exact transition rule from %d to %d"),
                   (int32)FromState, (int32)ToState);
            return &Rule;
        }

        if (
            Rule.FromState == EAnimationKey::None &&
            !WildcardMatch
        )
        {
            UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem: Found wildcard transition rule to %d"),
                   (int32)ToState);
            WildcardMatch = &Rule;
        }
    }

    if (WildcardMatch)
    {
        return WildcardMatch;
    }

    UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem: No transition rule found from %d to %d"),
           (int32)FromState, (int32)ToState);

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
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem::AdvanceQueue - Actor is null"));
        return;
    }

    if (State.PendingQueue.Num() == 0)
    {
        UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem::AdvanceQueue - No pending states for %s"),
               *Actor->GetName());
        return;
    }

    const EAnimationKey NextKey =
        State.PendingQueue[0];

    State.PendingQueue.RemoveAt(0);

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem::AdvanceQueue - Playing animation %d for %s (remaining: %d)"),
           (int32)NextKey, *Actor->GetName(), State.PendingQueue.Num());

    UAnimMontage* Montage = nullptr;

    const bool bPlayed =
        PlayStateAnimation(
            Actor,
            NextKey,
            Montage
        );

    if (!bPlayed)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystem::AdvanceQueue - Failed to play animation %d for %s"),
               (int32)NextKey, *Actor->GetName());

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
            UE_LOG(LogTemp, Log, TEXT("AnimationSystem::AdvanceQueue - Advancing to next state in queue for %s"),
                   *Actor->GetName());
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

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem::AdvanceQueue - Successfully played animation %d for %s"),
           (int32)NextKey, *Actor->GetName());

    if (!Montage)
    {
        // Fallback playback path — no montage to bind an end delegate to.
        return;
    }

    USkeletalMeshComponent* Mesh =
        Actor->FindComponentByClass<USkeletalMeshComponent>();

    if (!Mesh)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem::AdvanceQueue - No SkeletalMeshComponent found on %s"),
               *Actor->GetName());
        return;
    }

    UAnimInstance* AnimInstance =
        Mesh->GetAnimInstance();

    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem::AdvanceQueue - No AnimInstance found on %s"),
               *Actor->GetName());
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
    UAnimMontage*& OutMontage
)
{
    OutMontage = nullptr;

    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem::PlayStateAnimation - Actor is null"));
        return false;
    }

    UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem::PlayStateAnimation - Playing key %d for %s"),
           (int32)Key, *Actor->GetName());

    UAnimationRegistryComponent* Registry =
        FindAnimationRegistry(Actor);

    if (!Registry)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem::PlayStateAnimation - No AnimationRegistryComponent found on %s"),
               *Actor->GetName());
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
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem::PlayStateAnimation - Animation not found for key %d on %s"),
               (int32)Key, *Actor->GetName());
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem::PlayStateAnimation - Found animation for key %d on %s"),
           (int32)Key, *Actor->GetName());

    USkeletalMeshComponent* Mesh =
        Actor->FindComponentByClass<USkeletalMeshComponent>();

    if (!Mesh)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem::PlayStateAnimation - No SkeletalMeshComponent found on %s"),
               *Actor->GetName());
        return false;
    }

    UAnimInstance* AnimInstance =
        Mesh->GetAnimInstance();

    //=========================================================================
    // AnimBP / Dynamic Montage
    //=========================================================================

    if (AnimInstance)
    {
        UE_LOG(LogTemp, Log, TEXT("AnimationSystem::PlayStateAnimation - Using AnimInstance for %s"),
               *Actor->GetName());

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
                Definition.PlaybackMode ==
                    EAnimationPlaybackMode::Loop

                    ? EffectiveInfiniteLoopCount

                    : FMath::Max(
                        1,
                        Definition.LoopCount
                    );

            OutMontage = AnimInstance
                ->PlaySlotAnimationAsDynamicMontage(
                    SequenceBase,

                    ResolveSlotName(Definition.Slot),

                    Definition.BlendInTime,
                    Definition.BlendOutTime,
                    Definition.PlayRate,
                    LoopCount,

                    /* BlendOutTriggerTime */
                    -1.0f,

                    Definition.StartPosition
                );

            if (OutMontage)
            {
                if (ACharacterActor* CharacterActor = Cast<ACharacterActor>(Actor))
                {
                    CharacterActor->SyncPoseableMeshFromSkeletalMesh();
                }
                UE_LOG(LogTemp, Log, TEXT("AnimationSystem::PlayStateAnimation - Successfully created dynamic montage for %s"),
                       *Actor->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("AnimationSystem::PlayStateAnimation - Failed to create dynamic montage for %s"),
                       *Actor->GetName());
            }

            return OutMontage != nullptr;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AnimationSystem::PlayStateAnimation - Animation is not a SequenceBase for %s"),
                   *Actor->GetName());
            return false;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("AnimationSystem::PlayStateAnimation - No AnimInstance found for %s, using fallback"),
           *Actor->GetName());

    //=========================================================================
    // Fallback
    //=========================================================================

    const bool bLooping =
        Definition.PlaybackMode ==
        EAnimationPlaybackMode::Loop;

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem::PlayStateAnimation - Using fallback playback for %s (looping: %d)"),
           *Actor->GetName(), bLooping);

    Mesh->PlayAnimation(
        Definition.Animation,
        bLooping
    );

    if (ACharacterActor* CharacterActor = Cast<ACharacterActor>(Actor))
    {
        CharacterActor->SyncPoseableMeshFromSkeletalMesh();
    }

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
            UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem::OnStateAnimationEnded - Animation interrupted"));
        }
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem::OnStateAnimationEnded - Animation ended for %s, checking queue"),
           *Actor->GetName());

    FActorAnimationState* State =
        FindActorAnimationState(
            Actor.Get(),
            Layer
        );

    if (!State)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystem::OnStateAnimationEnded - No state found for %s"),
               *Actor->GetName());
        return;
    }

    // A newer request may have already replaced
    // this montage on this specific layer.
    if (State->ActiveMontage.Get() != Montage)
    {
        UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem::OnStateAnimationEnded - Montage has been replaced for %s"),
               *Actor->GetName());
        return;
    }

    if (State->PendingQueue.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("AnimationSystem::OnStateAnimationEnded - Advancing to next state for %s"),
               *Actor->GetName());
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
            UE_LOG(
                LogTemp,
                Log,
                TEXT("AnimationSystem::OnStateAnimationEnded - Clearing finished overlay layer %d for %s"),
                static_cast<int32>(Layer),
                *Actor->GetName()
            );
            State->CurrentState = EAnimationKey::None;
            State->ActiveMontage.Reset();
            State->StateEnteredTime = 0.0;
        }
        else
        {
            UE_LOG(LogTemp, Verbose, TEXT("AnimationSystem::OnStateAnimationEnded - No pending states for %s"),
                   *Actor->GetName());
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
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem::OnAnimationRequest - Invalid Actor in payload"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AnimationSystem::OnAnimationRequest - Received request for %s: State=%d, bForce=%d, Layer=%d"),
           *Actor->GetName(),
           (int32)Payload.RequestedState,
           Payload.bForce,
           (int32)Payload.Layer);

    RequestAnimationState(
        Actor,
        Payload.RequestedState,
        Payload.bForce,
        Payload.Layer
    );
}
