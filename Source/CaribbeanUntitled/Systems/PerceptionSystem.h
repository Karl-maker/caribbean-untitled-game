#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PerceptionSystem.generated.h"

class UCharacterPerceptionComponent;
class UStatsComponent;
struct FPerceivedActor;

/**
 * Owns ALL perception computation. UCharacterPerceptionComponent is
 * pure data (settings + results) - it has no logic. This system:
 *
 *  1. Caches "who is physically nearby" spatial queries so that
 *     perceivers standing close together share one query result
 *     instead of each running their own overlap (grouping/caching).
 *  2. For each perceiver, checks that shared candidate list against
 *     ITS OWN sight range/angle (cone-of-vision) and then does a
 *     line-of-sight trace (walls/crates/anything blocking).
 *  3. Applies the keep/forget rules for actors that leave view.
 *  4. Writes the results back onto each character's
 *     UCharacterPerceptionComponent.
 */
UCLASS()
class CARIBBEANUNTITLED_API UPerceptionSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    // ---- grouping / caching config ----

    /**
     * Size (uu) of one grouping cell. Perceivers within roughly
     * this distance of each other end up sharing a cached query
     * result. Default 500uu (~5m).
     */
    UPROPERTY(EditAnywhere, Category = "Perception|Grouping")
    float CellSize = 500.0f;

    /**
     * How long (seconds) a cell's cached "nearby actors" result
     * stays valid before it's re-queried. This is what makes the
     * spatial query run "every so often" rather than constantly.
     */
    UPROPERTY(EditAnywhere, Category = "Perception|Grouping")
    float CacheLifetime = 0.5f;

    /**
     * Extra radius padded onto the query so actors near a cell's
     * edge still get caught.
     */
    UPROPERTY(EditAnywhere, Category = "Perception|Grouping")
    float QueryPadding = 200.0f;

    /**
     * Object types considered perceivable. Defaults to Pawn +
     * WorldDynamic (dropped weapons/tools/pickups are usually
     * WorldDynamic AActors, not Pawns). For tighter control, set up
     * a dedicated custom Object Channel (e.g. "Perceivable") in
     * Project Settings > Collision, assign it to both characters
     * and item actors, and point this array at just that channel
     * so perception doesn't sweep in unrelated WorldDynamic debris.
     */
    UPROPERTY(EditAnywhere, Category = "Perception|Grouping")
    TArray<TEnumAsByte<EObjectTypeQuery>> PerceivableObjectTypes;

    UPerceptionSystem();

    /**
     * Call this once per frame (from whatever drives your NPCs - a
     * manager, a game mode, an AI subsystem, etc.) with the world
     * and delta time. Internally, every
     * UCharacterPerceptionComponent in the world is only actually
     * re-evaluated on its own PerceptionUpdateInterval cadence -
     * most are skipped on most calls.
     */
    void Tick(UWorld* World, float DeltaTime);

private:

    struct FPerceptionCell
    {
        TArray<TWeakObjectPtr<AActor>> Actors;
        double LastQueryTime = -1000.0;
        float QueriedRadius = 0.0f;
    };

    FIntVector LocationToCell(const FVector& Location) const;

    /** Shared/cached "what's physically nearby" query - the grouping piece. */
    const TArray<TWeakObjectPtr<AActor>>& GetNearbyActors(const FVector& Location, float QueryRadius);

    /** Evaluates one perceiver against the shared nearby list and writes results onto its component. */
    void EvaluatePerceiver(
        UWorld* World,
        AActor* Owner,
        UCharacterPerceptionComponent* Perception,
        UStatsComponent* Stats);

    bool IsInSightCone(
        const FVector& EyeLocation,
        const FVector& Forward,
        const FVector& TargetLocation,
        float SightRange,
        float SightAngleDegrees,
        float& OutDistance) const;

    /** Occlusion check - is a wall/crate/anything else physically blocking the view. */
    bool HasLineOfSight(
        UWorld* World,
        AActor* Owner,
        const FVector& EyeLocation,
        AActor* Target,
        ECollisionChannel TraceChannel) const;

    TMap<FIntVector, FPerceptionCell> Cells;
};
