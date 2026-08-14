//
// InteractiveComponent.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractiveComponent.generated.h"


/**
 * Types of interactions an Actor can perform.
 */
UENUM(BlueprintType)
enum class EInteractiveType : uint8
{
    None    UMETA(DisplayName = "None"),
    Mount   UMETA(DisplayName = "Mount"),
    PickUp  UMETA(DisplayName = "Pick Up")
};


/**
 * Stores interaction state for an Actor.
 *
 * This component does NOT perform the interaction.
 *
 * It stores:
 * - What type of interaction is requested
 * - Which Actor is being interacted with
 * - Whether an interaction is currently being requested
 * - Interaction range
 * - Interaction progress
 *
 * InteractiveSystem reads this component and performs the
 * actual gameplay logic.
 */
UCLASS(
    ClassGroup=(Gameplay),
    BlueprintType,
    Blueprintable,
    meta=(BlueprintSpawnableComponent)
)
class CARIBBEANUNTITLED_API UInteractiveComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UInteractiveComponent();


    //=========================================================================
    // Interaction State
    //=========================================================================

    /**
     * Type of interaction currently being requested.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Interactive"
    )
    EInteractiveType InteractionType = EInteractiveType::None;


    /**
     * Whether this Actor is currently requesting an interaction.
     *
     * This is generally set by Control/Input/AI systems and consumed
     * by the InteractiveSystem.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Interactive"
    )
    bool bInteractionRequested = false;


    /**
     * Actor currently being interacted with.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Interactive"
    )
    TObjectPtr<AActor> InteractionTarget = nullptr;


    /**
     * Maximum distance at which an interaction can occur.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Interactive",
        meta = (ClampMin = "0.0")
    )
    float InteractionRange = 200.0f;


    /**
     * Optional interaction duration.
     *
     * 0.0 = instant interaction.
     * > 0.0 = interaction takes this many seconds.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Interactive",
        meta = (ClampMin = "0.0")
    )
    float InteractionDuration = 0.0f;


    /**
     * Current progress of the interaction.
     *
     * 0.0 = not started
     * 1.0 = complete
     */
    UPROPERTY(
        BlueprintReadOnly,
        Category = "Interactive",
        meta = (ClampMin = "0.0", ClampMax = "1.0")
    )
    float InteractionProgress = 0.0f;


    //=========================================================================
    // Interaction Requests
    //=========================================================================

    /**
     * Requests an interaction with a target.
     */
    UFUNCTION(BlueprintCallable, Category = "Interactive")
    void RequestInteraction(
        EInteractiveType Type,
        AActor* Target
    );


    /**
     * Clears the current interaction request.
     */
    UFUNCTION(BlueprintCallable, Category = "Interactive")
    void ClearInteraction();


    /**
     * Sets the Actor being interacted with.
     */
    UFUNCTION(BlueprintCallable, Category = "Interactive")
    void SetInteractionTarget(AActor* Target);


    /**
     * Gets the current interaction target.
     */
    UFUNCTION(BlueprintPure, Category = "Interactive")
    AActor* GetInteractionTarget() const
    {
        return InteractionTarget;
    }


    /**
     * Sets the interaction type.
     */
    UFUNCTION(BlueprintCallable, Category = "Interactive")
    void SetInteractionType(EInteractiveType Type);


    /**
     * Gets the current interaction type.
     */
    UFUNCTION(BlueprintPure, Category = "Interactive")
    EInteractiveType GetInteractionType() const
    {
        return InteractionType;
    }


    //=========================================================================
    // State Queries
    //=========================================================================

    /**
     * Returns true if an interaction has been requested.
     */
    UFUNCTION(BlueprintPure, Category = "Interactive")
    bool HasInteractionRequest() const
    {
        return bInteractionRequested;
    }


    /**
     * Returns true if there is a valid interaction target.
     */
    UFUNCTION(BlueprintPure, Category = "Interactive")
    bool HasInteractionTarget() const
    {
        return IsValid(InteractionTarget);
    }


    /**
     * Returns true if the Actor is currently performing an interaction.
     */
    UFUNCTION(BlueprintPure, Category = "Interactive")
    bool IsInteracting() const
    {
        return InteractionProgress > 0.0f;
    }


    /**
     * Returns true if the interaction has completed.
     */
    UFUNCTION(BlueprintPure, Category = "Interactive")
    bool IsInteractionComplete() const
    {
        return InteractionProgress >= 1.0f;
    }


    //=========================================================================
    // Range
    //=========================================================================

    UFUNCTION(BlueprintCallable, Category = "Interactive")
    void SetInteractionRange(float Range);


    UFUNCTION(BlueprintPure, Category = "Interactive")
    float GetInteractionRange() const
    {
        return InteractionRange;
    }


    /**
     * Checks whether the owning Actor is within interaction range
     * of the current target.
     */
    UFUNCTION(BlueprintPure, Category = "Interactive")
    bool IsTargetInRange() const;


    //=========================================================================
    // Progress
    //=========================================================================

    /**
     * Sets interaction progress.
     *
     * Value is clamped between 0 and 1.
     */
    UFUNCTION(BlueprintCallable, Category = "Interactive")
    void SetInteractionProgress(float Progress);


    UFUNCTION(BlueprintPure, Category = "Interactive")
    float GetInteractionProgress() const
    {
        return InteractionProgress;
    }


    /**
     * Resets interaction progress to zero.
     */
    UFUNCTION(BlueprintCallable, Category = "Interactive")
    void ResetInteractionProgress();


protected:

    virtual void BeginPlay() override;
};
