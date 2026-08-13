//
//  ControlComponent.h
//  CaribbeanUntitled
//
//  Created by Karl-Johan Bailey on 8/12/26.
//

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ControlComponent.generated.h"


/**
 * Defines where control intent originates.
 */
UENUM(BlueprintType)
enum class EControlType : uint8
{
    Player  UMETA(DisplayName = "Player"),
    AI      UMETA(DisplayName = "AI"),
    Network UMETA(DisplayName = "Network"),
    Script  UMETA(DisplayName = "Script")
};


/**
 * Individual gameplay intents.
 *
 * Values are represented as weights from -1.0 to 1.0 where appropriate.
 */
UENUM(BlueprintType)
enum class EControlIntent : uint8
{
    Forward,
    Back,
    Left,
    Right,
    Jump,
    Interact
};


/**
 * Stores normalized control intent for an Actor.
 *
 * The component does not perform movement, jumping, interaction,
 * or other gameplay logic.
 *
 * It only represents what the Actor intends to do.
 *
 * Example:
 *
 * Player input:
 *     Move = (0.0, 1.0)
 *
 * AI input:
 *     Move = (0.5, 0.8)
 *
 * Network input:
 *     Move = (-1.0, 0.0)
 *
 * Gameplay systems consume this component.
 */
UCLASS(ClassGroup=(Gameplay), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UControlComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UControlComponent();

    /**
     * Determines the source of this control.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Control"
    )
    EControlType ControlType = EControlType::Player;


    /**
     * Normalized movement intent.
     *
     * X:
     *     -1 = Left
     *      0 = None
     *     +1 = Right
     *
     * Y:
     *     -1 = Back
     *      0 = None
     *     +1 = Forward
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Control|Movement",
        meta = (
            ClampMin = "-1.0",
            ClampMax = "1.0"
        )
    )
    FVector2D MoveIntent = FVector2D::ZeroVector;


    /**
     * Jump intent.
     *
     * 0 = no intent
     * 1 = full intent
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Control|Action",
        meta = (
            ClampMin = "0.0",
            ClampMax = "1.0"
        )
    )
    float JumpIntent = 0.0f;


    /**
     * Interaction intent.
     *
     * 0 = no intent
     * 1 = full intent
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Control|Action",
        meta = (
            ClampMin = "0.0",
            ClampMax = "1.0"
        )
    )
    float InteractIntent = 0.0f;
    
    /**
     * Look input / mouse delta.
     *
     * X = horizontal look input
     * Y = vertical look input
     *
     * This is input rather than a world-space direction.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Control|Look"
    )
    FVector2D LookInput = FVector2D::ZeroVector;

    
    /**
     * Desired world-space look direction.
     *
     * This represents the direction the controlled Actor wants
     * to look toward, independent of where the input came from.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Control|Look"
    )
    FVector LookDirection = FVector::ForwardVector;
    
    
    /**
     * Desired world-space movement direction.
     *
     * This is independent of the control source.
     * Player, AI, Network, and Script can all provide it.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Control|Movement"
    )
    FVector MoveDirection = FVector::ForwardVector;

public:

    /**
     * Sets the complete movement intent.
     *
     * X = Left / Right
     * Y = Back / Forward
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void SetMoveIntent(const FVector2D& Intent);


    /**
     * Sets forward intent.
     *
     * Positive values move forward.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void SetForwardIntent(float Weight);


    /**
     * Sets backward intent.
     *
     * Positive values move backward.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void SetBackIntent(float Weight);


    /**
     * Sets left intent.
     *
     * Positive values move left.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void SetLeftIntent(float Weight);


    /**
     * Sets right intent.
     *
     * Positive values move right.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void SetRightIntent(float Weight);


    /**
     * Sets jump intent.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void SetJumpIntent(float Weight);


    /**
     * Sets interaction intent.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void SetInteractIntent(float Weight);


    /**
     * Gets the weight associated with an intent.
     */
    UFUNCTION(BlueprintPure, Category = "Control")
    float GetIntentWeight(EControlIntent Intent) const;


    /**
     * Sets the weight associated with an intent.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void SetIntentWeight(
        EControlIntent Intent,
        float Weight
    );


    /**
     * Returns true if the specified intent is active.
     */
    UFUNCTION(BlueprintPure, Category = "Control")
    bool HasIntent(
        EControlIntent Intent,
        float MinimumWeight = 0.01f
    ) const;


    /**
     * Returns whether any movement intent exists.
     */
    UFUNCTION(BlueprintPure, Category = "Control")
    bool HasMovementIntent() const;


    /**
     * Returns whether any action intent exists.
     */
    UFUNCTION(BlueprintPure, Category = "Control")
    bool HasActionIntent() const;


    /**
     * Clears all control intent.
     *
     * Useful when a system consumes input each frame.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void ClearIntents();


    /**
     * Clears only movement intent.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void ClearMovementIntent();


    /**
     * Clears only action intent.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void ClearActionIntent();


    /**
     * Sets mouse/controller look input.
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Look")
    void SetLookInput(const FVector2D& Input);


    /**
     * Sets the desired world-space look direction.
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Look")
    void SetLookDirection(const FVector& Direction);


    /**
     * Returns the desired look direction.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Look")
    FVector GetLookDirection() const;


    /**
     * Returns the current look input.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Look")
    FVector2D GetLookInput() const;


    /**
     * Clears look input.
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Look")
    void ClearLookInput();
    
    
    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetMoveDirection(const FVector& Direction);

    UFUNCTION(BlueprintPure, Category = "Control|Movement")
    FVector GetMoveDirection() const;
    
protected:

    virtual void BeginPlay() override;
};
