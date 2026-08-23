#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TimerManager.h"
#include "Tickable.h"

#include "StaminaRegenerationSystem.generated.h"

class UStatsComponent;
class UActorRegistry;

/**
 * Handles stamina regeneration for actors with a StatsComponent.
 *
 * This system uses a tick that runs at the configured interval.
 *
 * The regeneration tick:
 * - Runs at 4 checks per second (configurable).
 * - Only processes actors that need stamina regeneration.
 * - Gets actors from ActorRegistry with StatsComponent.
 */
UCLASS()
class CARIBBEANUNTITLED_API UStaminaRegenerationSystem : public UObject
{
    GENERATED_BODY()

public:
    void Tick(UWorld* World, float DeltaTime);
    TStatId GetStatId() const;
    bool IsTickable() const { return true; }

    /**
     * Called when an actor consumes stamina.
     *
     * Resets the regeneration delay for this actor.
     */
    void RequestRegeneration(UStatsComponent* Stats);

    /**
     * Removes an actor from stamina regeneration processing.
     */
    void StopRegeneration(UStatsComponent* Stats);

    /**
     * Cleans up the system.
     */
    void Cleanup();

    /**
     * How often regeneration is checked in seconds.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float RegenerationInterval = 0.25f;

private:

    /**
     * Processes stamina regeneration for a single actor.
     */
    void ProcessActor(AActor* Actor, float DeltaTime);

    /**
     * Time accumulator for tick-based regeneration.
     */
    float AccumulatedTime = 0.0f;

    /**
     * Actors currently waiting for or receiving stamina regeneration.
     *
     * The float is the remaining regeneration delay.
     */
    TMap<TWeakObjectPtr<UStatsComponent>, float> RegeneratingActors;
};
