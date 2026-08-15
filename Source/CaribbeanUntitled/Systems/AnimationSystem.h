//
// AnimationSystem.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "../Events/AnimationRequestPayload.h"
#include "AnimationSystem.generated.h"

class UAnimationRegistryComponent;
class UAnimInstance;
class UAnimMontage;
class UEventBusSubsystem;
class UActorRegistry;
struct FAnimationDefinition;


/**
 * Generic, event-driven animation playback system.
 *
 * Plays any EAnimationKey for any actor that has an
 * UAnimationRegistryComponent, resolved through a per-actor state
 * machine so that a request like "Jump" while already in "Sprint" can
 * be strung into a distinct combo animation, and any transition can
 * be routed through intermediate animations (e.g. a landing pose
 * before settling back to Idle). This is the ONLY system in the game
 * that actually plays animations — everything else (movement,
 * combat, interaction, cutscenes...) just tells it what key it wants.
 *
 * Wiring:
 *
 *   - UActorRegistry supplies the set of actors that currently have an
 *     UAnimationRegistryComponent, and this system reads that
 *     component for asset/playback data per EAnimationKey.
 *
 *   - UEventBusSubsystem delivers FAnimationRequestPayload events.
 *     Nothing about input, velocity thresholds, combat rules, or
 *     anything else "why" lives here — this system only resolves
 *     "state X requested" into "play this sequence of animations".
 *     See UMovementAnimationSystem for the movement-specific system
 *     that translates player input into requests for this one.
 *
 * Extending:
 *
 *   - New animation keys: add to EAnimationKey (AnimationRegistryComponent.h)
 *     and register an FAnimationDefinition for it per-actor in the
 *     UAnimationRegistryComponent. No code change needed here.
 *
 *   - New transitions / combos / intermediate animations: add a row
 *     to TransitionRulesTable or TransitionRules. No code change
 *     needed here either — see FActorAnimTransitionRule for what a
 *     rule can express.
 */
UCLASS()
class CARIBBEANUNTITLED_API UAnimationSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    //=========================================================================
    // USubsystem
    //=========================================================================

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;


    //=========================================================================
    // Requests
    //=========================================================================

    /**
     * Requests that Actor's animation move to NewState.
     *
     * This is the same entry point that FAnimationRequestPayload
     * events on the EventBus are routed through, exposed directly so
     * C++ and Blueprint callers don't have to go via the bus if they
     * don't want to.
     *
     * Resolves any matching FActorAnimTransitionRule (combo overrides,
     * intermediate animations) and begins playback immediately.
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RequestAnimationState(
        AActor* Actor,
        EAnimationKey NewState,
        bool bForce = false,
        EAnimationLayer Layer = EAnimationLayer::Base
    );

    /**
     * Returns the animation state currently playing (or being
     * transitioned into) for Actor. EAnimationKey::None if the actor
     * has no tracked state.
     */
    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnimationKey GetCurrentState(AActor* Actor) const;

    /**
     * Stops tracking Actor. Call this from EndPlay/Destroyed if you
     * need to free state early; otherwise stale entries are pruned
     * lazily as invalid weak pointers are encountered.
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ClearActorState(AActor* Actor);


    //=========================================================================
    // Configuration
    //=========================================================================

    /**
     * Optional data table of FActorAnimTransitionRule rows. Designer
     * friendly, hot-reloadable way to add transitions/combos.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    TObjectPtr<UDataTable> TransitionRulesTable = nullptr;

    /**
     * Additional transition rules defined directly on this subsystem's
     * class defaults / config. Combined with TransitionRulesTable at
     * Initialize time. Prefer this for rules you want to keep close to
     * code (e.g. rules a gameplay programmer owns), and the data table
     * for rules a designer iterates on.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    TArray<FActorAnimTransitionRule> TransitionRules;


private:

    //=========================================================================
    // Per-actor state
    //=========================================================================

    /**
     * Tracks one actor's progress through a transition chain.
     */
    struct FActorAnimationState
    {
        /** State currently playing (or last state played, if idle). */
        EAnimationKey CurrentState = EAnimationKey::None;

        /** Remaining keys to play, in order, to finish the current chain. */
        TArray<EAnimationKey> PendingQueue;

        /** Montage currently driving CurrentState, if any. */
        TWeakObjectPtr<UAnimMontage> ActiveMontage;

        /** World time this actor's CurrentState was entered. */
        double StateEnteredTime = 0.0;
    };
    
    struct FActorAnimationLayers
    {
        TMap<EAnimationLayer, FActorAnimationState> Layers;
    };


    TMap<TWeakObjectPtr<AActor>, FActorAnimationLayers> ActorStates;

    FActorAnimationState& GetOrCreateActorState(
        AActor* Actor,
        EAnimationLayer Layer
    );
    
    FActorAnimationState* FindActorAnimationState(
        AActor* Actor,
        EAnimationLayer Layer
    );

    /** Removes entries whose actor has become invalid. */
    void PruneStaleActors();


    //=========================================================================
    // Transition resolution
    //=========================================================================

    /** All rules currently in effect (table rows + TransitionRules, merged at Initialize). */
    TArray<FActorAnimTransitionRule> ResolvedRules;

    void BuildResolvedRules();

    /**
     * Finds the best matching rule for FromState -> ToState.
     *
     * Prefers an exact FromState match; falls back to a wildcard rule
     * (FromState == None) applying to ToState. Combo rules
     * (OverrideResultState set) are skipped if TimeInFromState exceeds
     * their ComboWindowSeconds.
     */
    const FActorAnimTransitionRule* FindTransitionRule(
        EAnimationKey FromState,
        EAnimationKey ToState,
        double TimeInFromState
    ) const;


    //=========================================================================
    // Playback
    //=========================================================================

    /** Begins playing the next entry in State.PendingQueue, if any. */
    void AdvanceQueue(
        AActor* Actor,
        FActorAnimationState& State,
        EAnimationLayer Layer
    );

    /** Plays a single animation key on Actor and returns the montage it started, if any. */
    UAnimMontage* PlayStateAnimation(AActor* Actor, EAnimationKey Key);

    /** Bound to the active montage; advances the actor's queue when it finishes. */
    void OnStateAnimationEnded(
        UAnimMontage* Montage,
        bool bInterrupted,
        TWeakObjectPtr<AActor> Actor,
        EAnimationLayer Layer
    );


    //=========================================================================
    // Lookups
    //=========================================================================

    UAnimationRegistryComponent* FindAnimationRegistry(AActor* Actor) const;
    UEventBusSubsystem* GetEventBus() const;
    UActorRegistry* GetActorRegistry() const;

    /** Bus handler — translates FAnimationRequestPayload into RequestAnimationState. */
    void OnAnimationRequest(const FAnimationRequestPayload& Payload);
};
