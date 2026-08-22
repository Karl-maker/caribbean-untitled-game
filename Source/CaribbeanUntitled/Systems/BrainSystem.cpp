#include "BrainSystem.h"
#include "../Components/PerceptionComponent.h"
#include "../EventBus/EventBusSubsystem.h"
#include "FocusSystem.h"
#include "IKSystem.h"
#include "ActorRegistry.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

bool UBrainSystem::IsTickable() const
{
    return true; // cheap: registry lookup is a map find, not a world scan
}

void UBrainSystem::Cleanup()
{
    ThinkStates.Reset();
    FocusSystem = nullptr;
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
    if (!FocusSystem)
    {
        FocusSystem = NewObject<UFocusSystem>(this);
    }

    if (FocusSystem)
    {
        FocusSystem->Tick(DeltaTime);
    }

    if (!IKSystem)
    {
        IKSystem = NewObject<UIKSystem>(this);
    }

    if (IKSystem)
    {
        IKSystem->Tick(DeltaTime);
    }

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
        UE_LOG(LogTemp, Warning, TEXT("[Brain] NO BRAIN COMPONENT"));
        return Result;
    }

    const FBrainPersonality& P = BrainComponent->Personality;

    // --- Log what perception sees ---
    if (PerceptionComponent)
    {
        const TArray<FPerceivedActor>& PerceivedActors =
            PerceptionComponent->GetPerceivedActors();

        const FPerceivedActor* FirstActor =
            PerceivedActors.Num() > 0 ? &PerceivedActors[0] : nullptr;

        UE_LOG(LogTemp, Log, TEXT("[Brain] PERCEPTION DATA | Perceived=%d | FirstTarget=%s | FirstVisible=%s"),
            PerceivedActors.Num(),
            FirstActor && FirstActor->Actor ? *FirstActor->Actor->GetName() : TEXT("NULL"),
            FirstActor && FirstActor->bVisible ? TEXT("true") : TEXT("false")
        );
    }

    // No perception component at all - hold still, low-key idle.
    if (!PerceptionComponent)
    {
        Result.Intent = EBrainIntent::Idle;
        Result.Effort = 0.1f;
        Result.Feeling = EBrainFeeling::Neutral;
        UE_LOG(LogTemp, Log, TEXT("[Brain] NO PERCEPTION → IDLE"));
        return Result;
    }

    // --- A TARGET IS PERCEIVED - ALWAYS DO SOMETHING ---
    const TArray<FPerceivedActor>& PerceivedActors =
        PerceptionComponent->GetPerceivedActors();

    for (const FPerceivedActor& Perceived : PerceivedActors)
    {
        AActor* Target = Perceived.Actor;
        const bool bVisible = Perceived.bVisible;

        if (!Target)
        {
            continue;
        }

        UE_LOG(LogTemp, Warning, TEXT("[Brain] TARGET DETECTED! | Visible=%s | Target=%s | Personality: Sociability=%.2f, Curiosity=%.2f"),
            bVisible ? TEXT("true") : TEXT("false"),
            *Target->GetName(),
            P.Sociability,
            P.Curiosity
        );

        // If visible, always follow or at least look at the target
        if (bVisible && Target)
        {
            // Different behaviors based on personality, but ALWAYS react
            if (P.Sociability >= 0.4f)
            {
                Result.Intent = EBrainIntent::Follow;
                Result.Feeling = EBrainFeeling::Happy;
                Result.Effort = FMath::Clamp(0.3f + P.Sociability * 0.4f, 0.0f, 1.0f);
                UE_LOG(LogTemp, Warning, TEXT("[Brain] → FOLLOW (Sociable)"));
            }
            else if (P.Aggression >= 0.4f)
            {
                Result.Intent = EBrainIntent::Attack;
                Result.Feeling = EBrainFeeling::Angry;
                Result.Effort = FMath::Clamp(0.5f + P.Aggression * 0.5f, 0.0f, 1.0f);
                UE_LOG(LogTemp, Warning, TEXT("[Brain] → ATTACK (Aggressive)"));
            }
            else if (P.Curiosity >= 0.4f)
            {
                Result.Intent = EBrainIntent::Investigate;
                Result.Feeling = EBrainFeeling::Curious;
                Result.Effort = FMath::Clamp(0.2f + P.Curiosity * 0.5f, 0.0f, 1.0f);
                UE_LOG(LogTemp, Warning, TEXT("[Brain] → INVESTIGATE (Curious)"));
            }
            else
            {
                // DEFAULT: Always at least acknowledge the player
                Result.Intent = EBrainIntent::Follow; // Or Wander, or Idle - just DO something
                Result.Feeling = EBrainFeeling::Neutral;
                Result.Effort = 0.5f;
                UE_LOG(LogTemp, Warning, TEXT("[Brain] → FOLLOW (Default)"));
            }

            Result.TargetActor = Target;
            Result.TargetLocation = Target->GetActorLocation();
            return Result;
        }

        // Target was seen but now lost - investigate last known location
        if (!bVisible)
        {
            Result.Intent = EBrainIntent::Investigate;
            Result.Feeling = EBrainFeeling::Suspicious;
            Result.Effort = FMath::Clamp(0.2f + P.Curiosity * 0.5f, 0.0f, 1.0f);
            Result.TargetLocation = Perceived.LastKnownLocation;
            UE_LOG(LogTemp, Warning, TEXT("[Brain] → INVESTIGATE (Lost Target) | Location=(%.1f,%.1f,%.1f)"),
                Result.TargetLocation.X,
                Result.TargetLocation.Y,
                Result.TargetLocation.Z
            );
            return Result;
        }
    }

    // --- No target detected - wander or idle ---
    if (P.Curiosity >= 0.5f)
    {
        Result.Intent = EBrainIntent::Wander;
        Result.Feeling = EBrainFeeling::Curious;
        Result.Effort = FMath::Clamp(P.Curiosity * 0.4f, 0.1f, 0.6f);
        UE_LOG(LogTemp, Log, TEXT("[Brain] → WANDER"));
    }
    else
    {
        Result.Intent = EBrainIntent::Idle;
        Result.Feeling = EBrainFeeling::Neutral;
        Result.Effort = 0.1f;
        UE_LOG(LogTemp, Log, TEXT("[Brain] → IDLE"));
    }

    return Result;
}
