#include "StaminaRegenerationSystem.h"

#include "ActorRegistry.h"
#include "../Components/StatsComponent.h"

#include "Engine/World.h"


// ============================================================
// TICK
// ============================================================

void UStaminaRegenerationSystem::Tick(UWorld* World, float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    if (!World)
    {
        return;
    }

    UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();
    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors =
        Registry->GetActorsWithComponent<UStatsComponent>();

    if (!Actors)
    {
        return;
    }

    // Accumulate time to control regeneration frequency
    AccumulatedTime += DeltaTime;
    
    // Only process at the configured interval
    if (AccumulatedTime < RegenerationInterval)
    {
        return;
    }
    
    // Reset accumulator (keep remainder for precise timing)
    AccumulatedTime = 0.0f;

    // Process all actors with StatsComponent
    for (const TWeakObjectPtr<AActor>& WeakActor : *Actors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            ProcessActor(Actor, RegenerationInterval);
        }
    }
}

TStatId UStaminaRegenerationSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UStaminaRegenerationSystem, STATGROUP_Tickables);
}


// ============================================================
// PROCESS ACTOR
// ============================================================

void UStaminaRegenerationSystem::ProcessActor(AActor* Actor, float DeltaTime)
{
    if (!IsValid(Actor))
    {
        return;
    }

    UStatsComponent* Stats = Actor->FindComponentByClass<UStatsComponent>();
    if (!IsValid(Stats))
    {
        return;
    }

    // If this actor isn't in the regeneration map, skip
    if (!RegeneratingActors.Contains(Stats))
    {
        return;
    }

    // Already at maximum stamina - remove from tracking
    if (Stats->GetStamina() >= Stats->MaxStamina)
    {
        RegeneratingActors.Remove(Stats);
        return;
    }

    // ----------------------------------------------------
    // REGENERATION DELAY
    // ----------------------------------------------------

    float& TimeRemaining = RegeneratingActors[Stats];

    if (TimeRemaining > 0.0f)
    {
        TimeRemaining = FMath::Max(
            0.0f,
            TimeRemaining - DeltaTime
        );
        return; // Wait until delay expires
    }

    // ----------------------------------------------------
    // REGENERATE STAMINA
    // ----------------------------------------------------

    const float Amount = Stats->StaminaRegenRate * DeltaTime;
    Stats->RestoreStamina(Amount);

    // Reached maximum stamina - remove from tracking
    if (Stats->GetStamina() >= Stats->MaxStamina)
    {
        RegeneratingActors.Remove(Stats);
    }
}


// ============================================================
// REQUEST REGENERATION
// ============================================================

void UStaminaRegenerationSystem::RequestRegeneration(
    UStatsComponent* Stats)
{
    if (!IsValid(Stats))
    {
        return;
    }

    // Already full. There is nothing to regenerate.
    if (Stats->GetStamina() >= Stats->MaxStamina)
    {
        return;
    }

    // Add the actor if it isn't already being tracked.
    //
    // If it already exists, this resets the delay.
    RegeneratingActors.FindOrAdd(Stats) =
        Stats->StaminaRegenDelay;
}


// ============================================================
// STOP REGENERATION
// ============================================================

void UStaminaRegenerationSystem::StopRegeneration(
    UStatsComponent* Stats)
{
    if (!Stats)
    {
        return;
    }

    RegeneratingActors.Remove(Stats);
}


// ============================================================
// CLEANUP
// ============================================================

void UStaminaRegenerationSystem::Cleanup()
{
    RegeneratingActors.Empty();
}
