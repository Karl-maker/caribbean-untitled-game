#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tickable.h"
#include "../Components/CharacterBrainComponent.h"
#include "BrainSystem.generated.h"

class UCharacterBrainComponent;
class UCharacterPerceptionComponent;
class UActorRegistry;
class UFocusSystem;
class UIKSystem;

USTRUCT(BlueprintType)
struct FBrainIntentChangedPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Brain")
    TObjectPtr<AActor> Owner = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="Brain")
    EBrainIntent Intent = EBrainIntent::None;

    UPROPERTY(BlueprintReadOnly, Category="Brain")
    float Effort = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Brain")
    EBrainFeeling Feeling = EBrainFeeling::Neutral;

    UPROPERTY(BlueprintReadOnly, Category="Brain")
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="Brain")
    FVector TargetLocation = FVector::ZeroVector;
};


/**
 * Per-actor think-timing + last-broadcast state. The ActorRegistry
 * tells us WHICH actors have a brain; this is the state BrainSystem
 * itself still needs to own on top of that.
 */
USTRUCT()
struct FBrainThinkState
{
    GENERATED_BODY()

    float TimeUntilNextThink = 0.0f;

    FBrainIntentData LastBroadcastIntent;
    bool bHasBroadcastOnce = false;
};


/**
 * World subsystem that periodically re-evaluates each brain-having
 * actor's intent (via UActorRegistry's component index, not its own
 * registration list) and publishes a change event over the EventBus
 * only when something actually changed.
 *
 * Think intervals are jittered per-actor and re-rolled after every
 * think, so a room full of NPCs doesn't think in lockstep.
 */
UCLASS()
class CARIBBEANUNTITLED_API UBrainSystem : public UObject
{
    GENERATED_BODY()

public:
    void Cleanup();
    virtual void Tick(float DeltaTime);
    TStatId GetStatId() const;
    bool IsTickable() const;

    UPROPERTY(EditAnywhere, Category="Brain")
    float BaseThinkInterval = 0.5f;

    UPROPERTY(EditAnywhere, Category="Brain", meta=(ClampMin="0.0", ClampMax="1.0"))
    float ThinkIntervalJitter = 0.35f;

private:

    void ThinkFor(
        AActor* Actor,
        UCharacterBrainComponent* BrainComponent,
        FBrainThinkState& State,
        float DeltaTime
    );

    FBrainIntentData DecideIntent(
        const UCharacterBrainComponent* BrainComponent,
        const UCharacterPerceptionComponent* PerceptionComponent
    ) const;

    static bool HasRelevantChange(
        const FBrainIntentData& A,
        const FBrainIntentData& B
    );

    float RollNextThinkInterval() const;

    /** Lazily-populated, pruned each tick against the registry's current set. */
    TMap<TWeakObjectPtr<AActor>, FBrainThinkState> ThinkStates;

    UPROPERTY(Transient)
    TObjectPtr<UFocusSystem> FocusSystem;

    UPROPERTY(Transient)
    TObjectPtr<UIKSystem> IKSystem;
};
