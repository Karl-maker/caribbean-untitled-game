#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerceptionComponent.generated.h"

USTRUCT(BlueprintType)
struct CARIBBEANUNTITLED_API FPerceivedActor
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* Actor = nullptr;

    UPROPERTY(BlueprintReadWrite)
    float Distance = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bVisible = false;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastSeen = 0.0f;
};

/**
 * Pure data component: what this character CAN perceive (settings)
 * and what it currently DOES perceive (results). No logic lives
 * here - UPerceptionSystem (a world subsystem) reads the settings
 * below, does the actual cone/line-of-sight work, and writes the
 * results back through the setters here. Keeping this component
 * dumb means it stays reusable for anything else that just wants
 * to describe/read a character's perception state.
 */
UCLASS(ClassGroup=(Gameplay), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UCharacterPerceptionComponent
    : public UActorComponent
{
    GENERATED_BODY()

public:

    UCharacterPerceptionComponent();

    // ============================================================
    // RESULTS - written by UPerceptionSystem
    // ============================================================

    /**
     * Actors currently perceived by this character.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Perception"
    )
    TArray<FPerceivedActor> PerceivedActors;


    /**
     * Current perceived threats.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Perception"
    )
    TArray<FPerceivedActor> Threats;


    /**
     * The player currently perceived by this character.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Perception"
    )
    TObjectPtr<AActor> PerceivedPlayer = nullptr;


    /**
     * Last known location of the perceived player.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Perception"
    )
    FVector LastKnownPlayerLocation = FVector::ZeroVector;


    /**
     * Whether the player is currently visible.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Perception"
    )
    bool bCanSeePlayer = false;


    /**
     * Whether this character currently perceives a threat.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Perception"
    )
    bool bUnderThreat = false;


    // ============================================================
    // SETTINGS - describe what this character is capable of.
    // Read by UPerceptionSystem, never interpreted here.
    // ============================================================

    /** How often (seconds) UPerceptionSystem re-evaluates this character. */
    UPROPERTY(EditAnywhere, Category = "Perception|Sight")
    float PerceptionUpdateInterval = 0.2f;

    /** Below this much movement, a still-visible actor's entry is reused instead of rechecked. */
    UPROPERTY(EditAnywhere, Category = "Perception|Sight")
    float MinMovementToRecheck = 50.0f;

    /** How far a now-unseen actor must drift from its last known location before it's forgotten. */
    UPROPERTY(EditAnywhere, Category = "Perception|Sight")
    float ForgetDistance = 400.0f;

    /** How long (seconds) a now-unseen actor is remembered before it's forgotten outright. */
    UPROPERTY(EditAnywhere, Category = "Perception|Sight")
    float ForgetTime = 3.0f;

    /** Trace channel used for the line-of-sight occlusion check (walls, crates, etc). */
    UPROPERTY(EditAnywhere, Category = "Perception|Sight")
    TEnumAsByte<ECollisionChannel> SightTraceChannel = ECC_Visibility;


    // ============================================================
    // RUNTIME BOOKKEEPING - mutated by UPerceptionSystem
    // ============================================================

    /** Seconds accumulated since this character's last actual re-evaluation. Owned/updated by UPerceptionSystem. */
    UPROPERTY(VisibleInstanceOnly, Category = "Perception|Runtime")
    float TimeSinceLastUpdate = 0.0f;


public:

    void Clear();

    void SetPerceivedActors(
        const TArray<FPerceivedActor>& Actors
    );

    void SetThreats(
        const TArray<FPerceivedActor>& NewThreats
    );

    void SetPlayerPerception(
        AActor* Player,
        bool bVisible,
        const FVector& LastKnownLocation
    );

    const TArray<FPerceivedActor>& GetPerceivedActors() const
    {
        return PerceivedActors;
    }

    const TArray<FPerceivedActor>& GetThreats() const
    {
        return Threats;
    }

    AActor* GetPerceivedPlayer() const
    {
        return PerceivedPlayer;
    }

    bool HasPerceivedPlayer() const
    {
        return PerceivedPlayer != nullptr;
    }

    bool CanSeePlayer() const
    {
        return bCanSeePlayer;
    }

    bool IsUnderThreat() const
    {
        return bUnderThreat;
    }
};
