#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tickable.h"
#include "Math/RandomStream.h"
#include "../Components/CharacterBrainComponent.h"
#include "ActionSystem.generated.h"

class AActor;
class UControlComponent;
class UCharacterBrainComponent;
class UCharacterPerceptionComponent;
class UActorRegistry;


/**
 * Generic per-actor key/value scratch space so individual IActionBehaviors
 * can remember state (a wander target, a cooldown, etc) without the
 * shared FActionState struct needing a field for every behavior that
 * will ever exist. Add whatever key types you need as this grows.
 */
USTRUCT()
struct FActionBlackboard
{
    GENERATED_BODY()

    TMap<FName, float> Floats;
    TMap<FName, FVector> Vectors;

    float GetFloat(FName Key, float Default = 0.0f) const
    {
        const float* Found = Floats.Find(Key);
        return Found ? *Found : Default;
    }

    void SetFloat(FName Key, float Value)
    {
        Floats.Add(Key, Value);
    }

    FVector GetVector(FName Key, FVector Default = FVector::ZeroVector) const
    {
        const FVector* Found = Vectors.Find(Key);
        return Found ? *Found : Default;
    }

    void SetVector(FName Key, FVector Value)
    {
        Vectors.Add(Key, Value);
    }
};


/**
 * Everything a behavior needs to read the world and write control intent,
 * bundled so IActionBehavior::Execute doesn't grow a new parameter every
 * time a behavior needs another component.
 */
struct FActionContext
{
    AActor* Actor = nullptr;
    UControlComponent* Control = nullptr;
    const UCharacterBrainComponent* Brain = nullptr;
    const UCharacterPerceptionComponent* Perception = nullptr;
    float DeltaTime = 0.0f;

    FRandomStream& RandomStream;
    FActionBlackboard& Blackboard;

    FActionContext(FRandomStream& InRandomStream, FActionBlackboard& InBlackboard)
        : RandomStream(InRandomStream)
        , Blackboard(InBlackboard)
    {}
};


/**
 * A concrete behavior mapped to one or more EBrainIntent values.
 * Stateless by design - any per-actor memory it needs goes through
 * Context.Blackboard, not member fields, since one instance is shared
 * across every actor currently holding that intent.
 */
class CARIBBEANUNTITLED_API IActionBehavior
{
public:

    virtual ~IActionBehavior() = default;

    /** Called every action-tick while this behavior's intent is active. */
    virtual void Execute(FActionContext& Context) = 0;

    /** Called once when an actor's intent transitions INTO this behavior. */
    virtual void OnEnter(FActionContext& Context) {}

    /** Called once when an actor's intent transitions AWAY from this behavior. */
    virtual void OnExit(FActionContext& Context) {}

    /** Return > 0 to override the system's default action-tick interval for this behavior (e.g. Flee reacting faster than Wander). */
    virtual float GetPreferredInterval() const { return -1.0f; }
};


/** Per-actor bookkeeping the ActionSystem owns on top of the registry's set. */
struct FActionState
{
    float TimeUntilNextThink = 0.0f;
    EBrainIntent LastIntent = EBrainIntent::None;
    FRandomStream RandomStream;
    FActionBlackboard Blackboard;
    bool bInitialized = false;
};


/**
 * World subsystem that periodically translates each AI-controlled
 * actor's current brain intent into concrete UControlComponent intent
 * (locomotion only, for now) by dispatching to the IActionBehavior(s)
 * registered for that intent.
 *
 * Only processes actors whose UControlComponent::ControlType == AI -
 * player/network-controlled actors are left alone.
 *
 * Extend by calling AddBehavior() with a new EBrainIntent + IActionBehavior
 * from wherever makes sense (a module's Initialize, a game-specific
 * subsystem, etc) - UActionSystem itself never needs to change.
 */
UCLASS()
class CARIBBEANUNTITLED_API UActionSystem : public UObject
{
    GENERATED_BODY()

public:
    void Cleanup();
    void Tick(UWorld* World, float DeltaTime);
    TStatId GetStatId() const;
    bool IsTickable() const { return true; }

    /** Registers a behavior to run whenever an actor's brain intent equals Intent. Multiple behaviors may stack on one intent. */
    void AddBehavior(EBrainIntent Intent, TSharedPtr<IActionBehavior> Behavior);

    UPROPERTY(EditAnywhere, Category = "Action")
    float BaseActionInterval = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Action", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ActionIntervalJitter = 0.25f;

private:

    void BuildDefaultBehaviors();

    void ThinkFor(
        AActor* Actor,
        UControlComponent* Control,
        const UCharacterBrainComponent* Brain,
        const UCharacterPerceptionComponent* Perception,
        FActionState& State,
        float DeltaTime
    );

    void RunTransition(
        EBrainIntent OldIntent,
        EBrainIntent NewIntent,
        FActionContext& Context
    );

    float RollNextInterval(float PreferredOverride) const;

    TMap<EBrainIntent, TArray<TSharedPtr<IActionBehavior>>> Behaviors;
    TMap<TWeakObjectPtr<AActor>, FActionState> ActionStates;
};
