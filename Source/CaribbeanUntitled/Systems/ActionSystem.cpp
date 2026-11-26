#include "ActionSystem.h"
#include "ActorRegistry.h"
#include "../Components/ControlComponent.h"
#include "../Components/CombatComponent.h"
#include "../Components/PerceptionComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"


//=============================================================================
// Idle
//=============================================================================

class FIdleActionBehavior : public IActionBehavior
{
public:

    virtual void OnEnter(FActionContext& Context) override
    {
        if (Context.Control)
        {
            Context.Control->ClearMovementIntent();
        }
    }

    virtual void Execute(FActionContext& Context) override
    {
        // Locomotion-only for now: idle just holds still. Once non-locomotion
        // actions exist, this is where idle fidgets / in-place turns go.
    }
};


//=============================================================================
// Wander
//=============================================================================

namespace ActionBlackboardKeys
{
    static const FName WanderOrigin("WanderOrigin");
    static const FName WanderTarget("WanderTarget");
    static const FName NextWanderPickTime("NextWanderPickTime");
}

class FWanderActionBehavior : public IActionBehavior
{
public:

    float WanderRadius = 800.0f;
    float MinPickInterval = 2.0f;
    float MaxPickInterval = 6.0f;

    virtual void OnEnter(FActionContext& Context) override
    {
        if (!Context.Actor)
        {
            return;
        }

        Context.Blackboard.SetVector(ActionBlackboardKeys::WanderOrigin, Context.Actor->GetActorLocation());
        PickNewTarget(Context);
    }

    virtual void Execute(FActionContext& Context) override
    {
        if (!Context.Control || !Context.Actor)
        {
            return;
        }

        const UWorld* World = Context.Actor->GetWorld();
        const float NowSeconds = World ? World->GetTimeSeconds() : 0.0f;
        const float NextPickTime = Context.Blackboard.GetFloat(ActionBlackboardKeys::NextWanderPickTime);

        const FVector Target = Context.Blackboard.GetVector(ActionBlackboardKeys::WanderTarget, Context.Actor->GetActorLocation());
        const float AcceptRadiusSq = FMath::Square(Context.Control->GetDirectionAcceptanceRadius());
        const bool bReachedTarget = FVector::DistSquared(Context.Actor->GetActorLocation(), Target) <= AcceptRadiusSq;

        if (bReachedTarget || NowSeconds >= NextPickTime)
        {
            PickNewTarget(Context);
        }

        Context.Control->MoveToWorldLocation(Context.Blackboard.GetVector(ActionBlackboardKeys::WanderTarget));
    }

    virtual void OnExit(FActionContext& Context) override
    {
        if (Context.Control)
        {
            Context.Control->ClearDestination();
            Context.Control->ClearMovementIntent();
        }
    }

private:

    void PickNewTarget(FActionContext& Context)
    {
        if (!Context.Actor)
        {
            return;
        }

        const FVector Origin = Context.Blackboard.GetVector(ActionBlackboardKeys::WanderOrigin, Context.Actor->GetActorLocation());

        const float Angle = Context.RandomStream.FRandRange(0.0f, 2.0f * PI);
        const float Radius = Context.RandomStream.FRandRange(WanderRadius * 0.25f, WanderRadius);
        const FVector NewTarget = Origin + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

        Context.Blackboard.SetVector(ActionBlackboardKeys::WanderTarget, NewTarget);

        const UWorld* World = Context.Actor->GetWorld();
        const float NowSeconds = World ? World->GetTimeSeconds() : 0.0f;
        const float NextInterval = Context.RandomStream.FRandRange(MinPickInterval, MaxPickInterval);
        Context.Blackboard.SetFloat(ActionBlackboardKeys::NextWanderPickTime, NowSeconds + NextInterval);
    }
};

class FFollowActionBehavior : public IActionBehavior
{
public:
    virtual void Execute(FActionContext& Context) override
    {
        if (!Context.Actor || !Context.Control || !Context.Brain)
        {
            return;
        }

        AActor* Target = Context.Brain->GetIntent().TargetActor;
        if (!IsValid(Target))
        {
            Context.Control->ClearMovementIntent();
            return;
        }

        const FVector TargetLocation = Target->GetActorLocation();
        Context.Control->MoveToWorldLocation(TargetLocation);
        Context.Control->SetLookAtLocation(TargetLocation);
    }
};

class FInvestigateActionBehavior : public IActionBehavior
{
public:
    virtual void Execute(FActionContext& Context) override
    {
        if (!Context.Actor || !Context.Control || !Context.Brain)
        {
            return;
        }

        const FVector TargetLocation = Context.Brain->GetIntent().TargetLocation;
        Context.Control->MoveToWorldLocation(TargetLocation);
        Context.Control->SetLookAtLocation(TargetLocation);
    }
};

class FAttackActionBehavior : public IActionBehavior
{
public:
    virtual float GetPreferredInterval() const override { return 0.1f; }

    virtual void Execute(FActionContext& Context) override
    {
        if (!Context.Actor || !Context.Control || !Context.Brain)
        {
            return;
        }

        AActor* Target = Context.Brain->GetIntent().TargetActor;
        if (!IsValid(Target))
        {
            Context.Control->ClearMovementIntent();
            return;
        }

        const FVector TargetLocation = Target->GetActorLocation();
        Context.Control->SetLookAtLocation(TargetLocation);

        constexpr float AttackRange = 175.0f;
        if (FVector::DistSquared2D(Context.Actor->GetActorLocation(), TargetLocation) >
            FMath::Square(AttackRange))
        {
            Context.Control->MoveToWorldLocation(TargetLocation);
            return;
        }

        Context.Control->ClearMovementIntent();
        if (UCombatComponent* Combat =
            Context.Actor->FindComponentByClass<UCombatComponent>())
        {
            if (Combat->CanAttack())
            {
                Combat->RequestAttack(ECombatAction::LightAttack);
            }
        }
    }
};


//=============================================================================
// Flee
//=============================================================================

class FFleeActionBehavior : public IActionBehavior
{
public:

    // Fleeing is urgent - react faster than the system's default interval.
    virtual float GetPreferredInterval() const override { return 0.1f; }

    virtual void Execute(FActionContext& Context) override
    {
        if (!Context.Control || !Context.Actor)
        {
            return;
        }

        const AActor* ThreatActor = Context.Brain ? Context.Brain->GetIntent().TargetActor : nullptr;
        const FVector ThreatLocation = ThreatActor
            ? ThreatActor->GetActorLocation()
            : (Context.Brain ? Context.Brain->GetIntent().TargetLocation : Context.Actor->GetActorLocation());

        // Stats-driven roll: braver characters are less likely to freeze
        // instead of actually moving away this tick. Copy this pattern
        // for any other "roll against a stat" variance later (hesitate
        // before interacting, flub an attack, etc).
        const float BaseFreezeChance = 0.25f;
        const float Bravery = Context.Brain ? Context.Brain->Personality.Bravery : 0.5f;
        const float FreezeChance = FMath::Clamp(BaseFreezeChance - (Bravery * BaseFreezeChance), 0.0f, BaseFreezeChance);

        if (Context.RandomStream.FRand() < FreezeChance)
        {
            Context.Control->SetWorldMoveDirection(FVector::ZeroVector);
            return;
        }

        FVector AwayDirection = Context.Actor->GetActorLocation() - ThreatLocation;
        AwayDirection.Z = 0.0f;

        if (!AwayDirection.Normalize())
        {
            // Threat is right on top of us - pick a random escape
            // direction rather than standing still.
            const float Angle = Context.RandomStream.FRandRange(0.0f, 2.0f * PI);
            AwayDirection = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
        }

        Context.Control->SetDirectionControlStrength(1.0f);
        Context.Control->SetWorldMoveDirection(AwayDirection);
    }

    virtual void OnExit(FActionContext& Context) override
    {
        if (Context.Control)
        {
            Context.Control->SetWorldMoveDirection(FVector::ZeroVector);
            Context.Control->SetDirectionControlStrength(0.0f);
        }
    }
};


//=============================================================================
// UActionSystem
//=============================================================================

UActionSystem::UActionSystem()
{
    BuildDefaultBehaviors();
}

TStatId UActionSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UActionSystem, STATGROUP_Tickables);
}

void UActionSystem::Cleanup()
{
    ActionStates.Reset();
    Behaviors.Reset();
}

void UActionSystem::BuildDefaultBehaviors()
{
    AddBehavior(EBrainIntent::Idle, MakeShared<FIdleActionBehavior>());
    AddBehavior(EBrainIntent::Wander, MakeShared<FWanderActionBehavior>());
    AddBehavior(EBrainIntent::Investigate, MakeShared<FInvestigateActionBehavior>());
    AddBehavior(EBrainIntent::Follow, MakeShared<FFollowActionBehavior>());
    AddBehavior(EBrainIntent::Flee, MakeShared<FFleeActionBehavior>());
    AddBehavior(EBrainIntent::Attack, MakeShared<FAttackActionBehavior>());
}

void UActionSystem::AddBehavior(EBrainIntent Intent, TSharedPtr<IActionBehavior> Behavior)
{
    if (!Behavior.IsValid())
    {
        return;
    }

    Behaviors.FindOrAdd(Intent).Add(MoveTemp(Behavior));
}

float UActionSystem::RollNextInterval(float PreferredOverride) const
{
    const float Base = (PreferredOverride > 0.0f) ? PreferredOverride : BaseActionInterval;
    const float Jitter = Base * ActionIntervalJitter;
    return FMath::Max(0.02f, Base + FMath::FRandRange(-Jitter, Jitter));
}

void UActionSystem::Tick(UWorld* World, float DeltaTime)
{
    UActorRegistry* Registry =
        World ? World->GetSubsystem<UActorRegistry>() : nullptr;
    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* BrainActors =
        Registry->GetActorsWithComponent<UCharacterBrainComponent>();

    if (!BrainActors || BrainActors->Num() == 0)
    {
        if (ActionStates.Num() > 0)
        {
            ActionStates.Reset();
        }
        return;
    }

    // Prune state for actors no longer registered.
    int32 PrunedCount = 0;
    for (auto It = ActionStates.CreateIterator(); It; ++It)
    {
        if (!It->Key.IsValid() || !BrainActors->Contains(It->Key))
        {
            It.RemoveCurrent();
            PrunedCount++;
        }
    }
    (void)PrunedCount;

    int32 ActorsProcessed = 0;
    int32 ActorsSkippedNoControl = 0;
    int32 ActorsSkippedNoBrain = 0;
    int32 ActorsWithControl = 0;

    for (const TWeakObjectPtr<AActor>& WeakActor : *BrainActors)
    {
        AActor* Actor = WeakActor.Get();
        if (!Actor)
        {
            continue;
        }

        UControlComponent* Control = Actor->FindComponentByClass<UControlComponent>();
        if (!Control)
        {
            ActorsSkippedNoControl++;
            continue;
        }

        if (Control->ControlType != EControlType::AI)
        {
            ActorsSkippedNoControl++;
            continue;
        }

        ActorsWithControl++;

        const UCharacterBrainComponent* Brain = Actor->FindComponentByClass<UCharacterBrainComponent>();
        const UCharacterPerceptionComponent* Perception = Actor->FindComponentByClass<UCharacterPerceptionComponent>();

        if (!Brain)
        {
            ActorsSkippedNoBrain++;
            continue;
        }

        // --- LOG: Found brain and control ---

        // Get current intent for logging
        EBrainIntent CurrentIntent = Brain->GetCurrentIntent();

        FActionState& State = ActionStates.FindOrAdd(WeakActor);

        if (!State.bInitialized)
        {
            State.RandomStream.Initialize(GetTypeHash(Actor));
            State.TimeUntilNextThink = RollNextInterval(-1.0f);
            State.bInitialized = true;
        }

        // --- LOG: State before thinking ---

        ThinkFor(Actor, Control, Brain, Perception, State, DeltaTime);
        ActorsProcessed++;

        // --- LOG: After thinking ---
    }

    // --- LOG: Summary ---
}

void UActionSystem::ThinkFor(
    AActor* Actor,
    UControlComponent* Control,
    const UCharacterBrainComponent* Brain,
    const UCharacterPerceptionComponent* Perception,
    FActionState& State,
    float DeltaTime)
{
    State.TimeUntilNextThink -= DeltaTime;
    if (State.TimeUntilNextThink > 0.0f)
    {
        return;
    }

    const EBrainIntent NewIntent = Brain->GetCurrentIntent();

    FActionContext Context(State.RandomStream, State.Blackboard);
    Context.Actor = Actor;
    Context.Control = Control;
    Context.Brain = Brain;
    Context.Perception = Perception;
    Context.DeltaTime = DeltaTime;

    if (NewIntent != State.LastIntent)
    {
        RunTransition(State.LastIntent, NewIntent, Context);
        State.LastIntent = NewIntent;
    }

    const TArray<TSharedPtr<IActionBehavior>>* ActiveBehaviors = Behaviors.Find(NewIntent);

    if (!ActiveBehaviors || ActiveBehaviors->Num() == 0)
    {
        // No behavior mapped for this intent yet - safe default is to
        // hold still rather than keep executing stale movement intent.
        Control->ClearMovementIntent();

        // Preferred interval isn't known without a behavior - use default.
        State.TimeUntilNextThink = RollNextInterval(-1.0f);
        return;
    }

    float PreferredInterval = -1.0f;

    for (const TSharedPtr<IActionBehavior>& Behavior : *ActiveBehaviors)
    {
        if (!Behavior.IsValid())
        {
            continue;
        }

        Behavior->Execute(Context);

        const float BehaviorPreferred = Behavior->GetPreferredInterval();
        if (BehaviorPreferred > 0.0f && (PreferredInterval < 0.0f || BehaviorPreferred < PreferredInterval))
        {
            PreferredInterval = BehaviorPreferred;
        }
    }

    State.TimeUntilNextThink = RollNextInterval(PreferredInterval);
}

void UActionSystem::RunTransition(
    EBrainIntent OldIntent,
    EBrainIntent NewIntent,
    FActionContext& Context)
{
    if (const TArray<TSharedPtr<IActionBehavior>>* OldBehaviors = Behaviors.Find(OldIntent))
    {
        for (const TSharedPtr<IActionBehavior>& Behavior : *OldBehaviors)
        {
            if (Behavior.IsValid())
            {
                Behavior->OnExit(Context);
            }
        }
    }

    if (const TArray<TSharedPtr<IActionBehavior>>* NewBehaviors = Behaviors.Find(NewIntent))
    {
        for (const TSharedPtr<IActionBehavior>& Behavior : *NewBehaviors)
        {
            if (Behavior.IsValid())
            {
                Behavior->OnEnter(Context);
            }
        }
    }
}
