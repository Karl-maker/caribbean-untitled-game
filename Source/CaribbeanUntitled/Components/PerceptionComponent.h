#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerceptionComponent.generated.h"


USTRUCT(BlueprintType)
struct FPerceivedActor
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Perception")
    TObjectPtr<AActor> Actor = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Perception")
    float Distance = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Perception")
    bool bVisible = false;
};


UCLASS(ClassGroup=(Gameplay), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UCharacterPerceptionComponent
    : public UActorComponent
{
    GENERATED_BODY()

public:

    UCharacterPerceptionComponent();

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
