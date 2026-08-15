//
// MovementAnimationTypes.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../Components/AnimationRegistryComponent.h" // EAnimationKey
#include "MovementAnimationTypes.generated.h"


// ============================================================================
// Event Payload
// ============================================================================

/**
 * Published on the EventBus whenever a gameplay system wants an
 * actor's animation to change directly, bypassing EControlIntent.
 *
 * Movement/locomotion driven by FMovementPayload (Sprint, Jump,
 * Interact, ...) is translated automatically — see
 * UMovementAnimationSystem::MapControlIntentToAnimationKey. This
 * payload is for everything EControlIntent doesn't express yet:
 * combat (Attack_1/2/3, Hit, Death), Mount/Dismount, or any other
 * system that wants to request a specific EAnimationKey without
 * going through control intents at all.
 *
 * Example:
 *
 *     EventBus->Publish(FMovementAnimationRequestPayload(
 *         PlayerActor,
 *         EAnimationKey::Sprint
 *     ));
 */
USTRUCT(BlueprintType)
struct CARIBBEANUNTITLED_API FMovementAnimationRequestPayload
{
    GENERATED_BODY()

    FMovementAnimationRequestPayload() = default;

    FMovementAnimationRequestPayload(
        AActor* InActor,
        EAnimationKey InRequestedState,
        bool bInForce = false
    )
        : Actor(InActor)
        , RequestedState(InRequestedState)
        , bForce(bInForce)
    {
    }

    /**
     * Actor whose animation state should change.
     */
    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    TObjectPtr<AActor> Actor = nullptr;

    /**
     * Gameplay animation key being requested.
     *
     * The MovementAnimationSystem decides, via the transition rules,
     * whether this is played directly, replaced by a combo/override
     * animation, or reached through intermediate animations.
     */
    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    EAnimationKey RequestedState = EAnimationKey::None;

    /**
     * If true, skips the "already in this state" no-op check and any
     * combo-window bookkeeping, and restarts the animation immediately.
     *
     * Useful for things like re-triggering a Hit reaction.
     */
    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bForce = false;
};


// ============================================================================
// Transition Rules
// ============================================================================

/**
 * Describes how to move from one animation state to another.
 *
 * Rules are looked up by (FromState, ToState). This is the single
 * extension point for the whole system: to add new movement/combat
 * animation behavior, add a rule here rather than touching the state
 * machine code.
 *
 * Three independent things a rule can express:
 *
 *   1. Simple passthrough (no rule matches)
 *        -> just play ToState directly.
 *
 *   2. Intermediate animations
 *        e.g. Sprint -> Idle plays [Land] before settling into Idle,
 *        or Fall -> Idle plays [Land] first.
 *
 *   3. Combo / string overrides
 *        e.g. Sprint -> Jump actually resolves to a distinct
 *        "sprint jump" animation key instead of the plain Jump key,
 *        as long as the request arrives within ComboWindowSeconds of
 *        entering Sprint. Attack_1 -> Attack_2 -> Attack_3 combos work
 *        the same way, chained across multiple rows.
 *
 * Rules can be authored in a UDataTable (design-friendly, hot
 * reloadable) and/or appended in code/Blueprint defaults via
 * UMovementAnimationSystem::TransitionRules.
 */
USTRUCT(BlueprintType)
struct CARIBBEANUNTITLED_API FMovementAnimTransitionRule : public FTableRowBase
{
    GENERATED_BODY()

    /**
     * State the actor must currently be in for this rule to apply.
     *
     * EAnimationKey::None acts as a wildcard: "from any state".
     * Exact-match rules are always preferred over wildcard rules.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EAnimationKey FromState = EAnimationKey::None;

    /**
     * The state being requested that this rule reacts to.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EAnimationKey ToState = EAnimationKey::None;

    /**
     * If set to anything other than None, this key is played instead
     * of ToState once the chain completes. This is what makes combo
     * strings work: e.g. FromState=Sprint, ToState=Jump,
     * OverrideResultState=SprintJump.
     *
     * Leave as None to just play ToState normally.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EAnimationKey OverrideResultState = EAnimationKey::None;

    /**
     * Animations played, in order, before the resolved result state
     * takes over as the actor's CurrentState. Each entry must have a
     * registered FAnimationDefinition in the actor's
     * UAnimationRegistryComponent.
     *
     * Example: Fall -> Idle with IntermediateStates = [Land].
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    TArray<EAnimationKey> IntermediateStates;

    /**
     * Only meaningful when OverrideResultState is set (combo rules).
     *
     * How long, after the actor entered FromState, the request is
     * still allowed to trigger this rule. If more time than this has
     * passed, the rule is skipped and the request falls through to a
     * wildcard rule (or a plain ToState transition).
     *
     * 0 = no time limit.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "0.0"))
    float ComboWindowSeconds = 0.0f;
};
