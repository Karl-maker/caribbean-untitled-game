#include "BrainSystem.h"
#include "../Components/PerceptionComponent.h"
#include "../EventBus/EventBusSubsystem.h"
#include "ActorRegistry.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

void UBrainSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ThinkStates.Reset();
}

void UBrainSystem::Deinitialize()
{
    ThinkStates.Reset();
    Super::Deinitialize();
}

bool UBrainSystem::IsTickable() const
{
    return true; // cheap: registry lookup is a map find, not a world scan
}

TStatId UBrainSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UBrainSystem, STATGROUP_Tickables);
}

float UBrainSystem::RollNextThinkInterval() const
{
    const float Jitter = BaseThinkInterval * ThinkIntervalJitter;
    return FMath::Max(0.05f, BaseThinkInterval + FMath::FRandRange(-Jitter, Jitter));
}

void UBrainSystem::Tick(float DeltaTime)
{
    UActorRegistry* Registry = GetWorld()
        ? GetWorld()->GetSubsystem<UActorRegistry>()
        : nullptr;

    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* BrainActors =
        Registry->GetActorsWithComponent<UCharacterBrainComponent>();

    if (!BrainActors || BrainActors->Num() == 0)
    {
        // Nothing has a brain right now - drop any stale state and bail.
        if (ThinkStates.Num() > 0)
        {
            ThinkStates.Reset();
        }
        return;
    }

    // Prune think-state entries for actors no longer in the registry's set
    // (despawned, or component removed).
    for (auto It = ThinkStates.CreateIterator(); It; ++It)
    {
        if (!It->Key.IsValid() || !BrainActors->Contains(It->Key))
        {
            It.RemoveCurrent();
        }
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *BrainActors)
    {
        AActor* Actor = WeakActor.Get();
        if (!Actor)
        {
            continue;
        }

        UCharacterBrainComponent* BrainComponent =
            Actor->FindComponentByClass<UCharacterBrainComponent>();

        if (!BrainComponent)
        {
            // Indexed under the component class but the component's gone -
            // registry will catch up; skip this frame.
            continue;
        }

        FBrainThinkState& State = ThinkStates.FindOrAdd(WeakActor);

        // First time we see this actor: stagger its first think too.
        if (!State.bHasBroadcastOnce && State.TimeUntilNextThink == 0.0f)
        {
            State.TimeUntilNextThink = RollNextThinkInterval();
        }

        ThinkFor(Actor, BrainComponent, State, DeltaTime);
    }
}

void UBrainSystem::ThinkFor(
    AActor* Actor,
    UCharacterBrainComponent* BrainComponent,
    FBrainThinkState& State,
    float DeltaTime)
{
    State.TimeUntilNextThink -= DeltaTime;
    if (State.TimeUntilNextThink > 0.0f)
    {
        return;
    }

    State.TimeUntilNextThink = RollNextThinkInterval();

    const UCharacterPerceptionComponent* PerceptionComponent =
        Actor->FindComponentByClass<UCharacterPerceptionComponent>();

    const FBrainIntentData NewIntent = DecideIntent(BrainComponent, PerceptionComponent);

    BrainComponent->SetIntent(NewIntent.Intent, NewIntent.Effort);
    BrainComponent->SetFeeling(NewIntent.Feeling);
    BrainComponent->SetTargetActor(NewIntent.TargetActor);
    BrainComponent->SetTargetLocation(NewIntent.TargetLocation);

    const bool bChanged = !State.bHasBroadcastOnce
        || HasRelevantChange(NewIntent, State.LastBroadcastIntent);

    if (!bChanged)
    {
        return;
    }

    State.LastBroadcastIntent = NewIntent;
    State.bHasBroadcastOnce = true;

    UWorld* World = Actor->GetWorld();
    UEventBusSubsystem* EventBus = World ? World->GetSubsystem<UEventBusSubsystem>() : nullptr;

    if (!EventBus)
    {
        return;
    }

    FBrainIntentChangedPayload Payload;
    Payload.Owner = Actor;
    Payload.Intent = NewIntent.Intent;
    Payload.Effort = NewIntent.Effort;
    Payload.Feeling = NewIntent.Feeling;
    Payload.TargetActor = NewIntent.TargetActor;
    Payload.TargetLocation = NewIntent.TargetLocation;

    EventBus->Publish<FBrainIntentChangedPayload>(Payload);
}

bool UBrainSystem::HasRelevantChange(
    const FBrainIntentData& A,
    const FBrainIntentData& B)
{
    constexpr float EffortTolerance = 0.02f;
    constexpr float LocationToleranceSq = 25.0f * 25.0f;

    if (A.Intent != B.Intent) return true;
    if (A.Feeling != B.Feeling) return true;
    if (A.TargetActor != B.TargetActor) return true;
    if (FMath::Abs(A.Effort - B.Effort) > EffortTolerance) return true;
    if (FVector::DistSquared(A.TargetLocation, B.TargetLocation) > LocationToleranceSq) return true;

    return false;
}

// @TODO - Add adjustments for enemies to see me as threat or anyone who follows me.
FBrainIntentData UBrainSystem::DecideIntent(
    const UCharacterBrainComponent* BrainComponent,
    const UCharacterPerceptionComponent* PerceptionComponent) const
{
    FBrainIntentData Result;

    if (!BrainComponent)
    {
        return Result;
    }

    const FBrainPersonality& P = BrainComponent->Personality;

    // No perception component at all - hold still, low-key idle.
    if (!PerceptionComponent)
    {
        Result.Intent = EBrainIntent::Idle;
        Result.Effort = 0.1f;
        Result.Feeling = EBrainFeeling::Neutral;
        return Result;
    }

    // --- Threats take priority, weighted by Bravery/Aggression. ---
    if (PerceptionComponent->IsUnderThreat() && PerceptionComponent->GetThreats().Num() > 0)
    {
        const FPerceivedActor& Threat = PerceptionComponent->GetThreats()[0];

        // Aggressive + brave characters fight; timid ones flee.
        const float FightScore = (P.Aggression * 0.6f) + (P.Bravery * 0.4f);

        if (FightScore >= 0.5f)
        {
            Result.Intent = EBrainIntent::Attack;
            Result.Feeling = EBrainFeeling::Angry;
            Result.Effort = FMath::Clamp(0.5f + P.Aggression * 0.5f, 0.0f, 1.0f);
        }
        else
        {
            Result.Intent = EBrainIntent::Flee;
            Result.Feeling = EBrainFeeling::Afraid;
            Result.Effort = FMath::Clamp(1.0f - P.Bravery * 0.4f, 0.3f, 1.0f);
        }

        Result.TargetActor = Threat.Actor;
        Result.TargetLocation = (Threat.bVisible && Threat.Actor)
            ? Threat.Actor->GetActorLocation()
            : Threat.LastKnownLocation;
        return Result;
    }

    // --- Player perception, weighted by Sociability/Curiosity. ---
    if (PerceptionComponent->HasPerceivedPlayer())
    {
        AActor* Player = PerceptionComponent->GetPerceivedPlayer();
        const bool bVisible = PerceptionComponent->CanSeePlayer();

        if (bVisible && Player && P.Sociability >= 0.4f)
        {
            Result.Intent = EBrainIntent::Follow;
            Result.Feeling = EBrainFeeling::Happy;
            Result.Effort = FMath::Clamp(0.3f + P.Sociability * 0.4f, 0.0f, 1.0f);
            Result.TargetActor = Player;
            Result.TargetLocation = Player->GetActorLocation();
            return Result;
        }

        if (!bVisible)
        {
            // Lost sight - go check the last known spot, curious
            // characters more insistently than others.
            Result.Intent = EBrainIntent::Investigate;
            Result.Feeling = EBrainFeeling::Suspicious;
            Result.Effort = FMath::Clamp(0.2f + P.Curiosity * 0.5f, 0.0f, 1.0f);
            Result.TargetLocation = PerceptionComponent->LastKnownPlayerLocation;
            return Result;
        }
    }

    // --- Nothing urgent: curious characters wander, others idle. ---
    if (P.Curiosity >= 0.5f)
    {
        Result.Intent = EBrainIntent::Wander;
        Result.Feeling = EBrainFeeling::Curious;
        Result.Effort = FMath::Clamp(P.Curiosity * 0.4f, 0.1f, 0.6f);
    }
    else
    {
        Result.Intent = EBrainIntent::Idle;
        Result.Feeling = EBrainFeeling::Neutral;
        Result.Effort = 0.1f;
    }

    return Result;
}
