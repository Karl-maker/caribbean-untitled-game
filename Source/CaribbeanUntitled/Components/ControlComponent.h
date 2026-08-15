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
 */
UENUM(BlueprintType)
enum class EControlIntent : uint8
{
    MoveForward,
    MoveBack,
    MoveLeft,
    MoveRight,

    Jump,
    Interact,
    Sprint,
    Crouch,
    Dash,
    Aim,
    Fire,
    Reload
};

/**
 * Stores normalized control intent for an Actor.
 *
 * Player movement:
 *
 * MoveIntent is LOCAL SPACE, relative to the Actor's current facing:
 *
 * X:  -1 = Left    0 = None    +1 = Right
 * Y:  -1 = Back    0 = None    +1 = Forward
 *
 *
 * External movement:
 *
 * WorldMoveDirection is an INDEPENDENT, world-space field. It is not
 * derived from MoveIntent and is not touched automatically by this
 * component's tick. Use it when an AI, script, or network system wants
 * the Actor to continuously move in a specific world-space direction.
 * Whoever sets it is responsible for clearing it (SetWorldMoveDirection
 * with a zero vector, or ClearMovementIntent) once external control
 * should end.
 *
 * DesiredWorldLocation / DirectionAcceptanceRadius can be used together
 * with MoveToWorldLocation() to drive WorldMoveDirection toward a target
 * point instead of setting a raw direction directly.
 *
 * DirectionControlStrength determines how strongly WorldMoveDirection
 * overrides MoveIntent when both are present:
 *
 * 0.0 = normal player movement (MoveIntent only)
 * 0.5 = 50% player / 50% external, blended
 * 1.0 = external movement completely controls direction
 *
 *
 * This component does not perform movement itself.
 * It only stores control intent — MovementSystem reads it and applies
 * the actual movement.
 */
UCLASS(
    ClassGroup=(Gameplay),
    BlueprintType,
    Blueprintable,
    meta=(BlueprintSpawnableComponent)
)
class CARIBBEANUNTITLED_API UControlComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UControlComponent();
    
    /**
     * How fast (degrees/sec) the Actor rotates to face LookDirection.
     * Higher = snappier turning. Used only when LookDirection is set.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Look", meta = (ClampMin = "0.0"))
    float LookRotationSpeed = 10.0f;

    //=========================================================================
    // Core Control Properties
    //=========================================================================

    /**
     * Determines the source of this control.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
    EControlType ControlType = EControlType::Player;


    //=========================================================================
    // Movement
    //=========================================================================

    /**
     * Movement intent in LOCAL space relative to the Actor's current
     * facing direction. Values are clamped to [-1, 1].
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Control|Movement",
        meta = (ClampMin = "-1.0", ClampMax = "1.0")
    )
    FVector2D MoveIntent = FVector2D::ZeroVector;

    /**
     * Direct world-space movement direction, set independently of
     * MoveIntent. See class comment above for full semantics.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Movement")
    FVector WorldMoveDirection = FVector::ZeroVector;

    /**
     * Desired world-space movement target, used by MoveToWorldLocation().
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Movement")
    FVector DesiredWorldLocation = FVector::ZeroVector;

    /**
     * Distance from DesiredWorldLocation at which the Actor is considered
     * to have reached the target (Unreal units).
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Control|Movement",
        meta = (ClampMin = "0.0")
    )
    float DirectionAcceptanceRadius = 50.0f;

    /**
     * How strongly WorldMoveDirection overrides MoveIntent when both are
     * non-zero. See class comment above.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Control|Movement",
        meta = (ClampMin = "0.0", ClampMax = "1.0")
    )
    float DirectionControlStrength = 1.0f;


    //=========================================================================
    // Look
    //=========================================================================

    /**
     * Desired world-space look direction (e.g. AI facing a target).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Look")
    FVector LookDirection = FVector::ForwardVector;

    /**
     * Raw look input from mouse/controller.
     * X = Horizontal look input / Yaw
     * Y = Vertical look input / Pitch
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Look")
    FVector2D LookInput = FVector2D::ZeroVector;


    //=========================================================================
    // Action Intents
    //=========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float JumpIntent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InteractIntent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SprintIntent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CrouchIntent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DashIntent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AimIntent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FireIntent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ReloadIntent = 0.0f;


    //=========================================================================
    // Movement Direction Setters / Getters
    //=========================================================================

    /**
     * Sets LookDirection by facing toward a world-space location.
     * Convenience wrapper around SetLookDirection for AI/script use
     * (e.g. face the player, face a patrol point).
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetLookAtLocation(const FVector& WorldLocation);
    
    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetMoveIntent(const FVector2D& Intent);

    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetWorldMoveDirection(const FVector& WorldDirection);

    UFUNCTION(BlueprintPure, Category = "Control|Movement")
    FVector GetWorldMoveDirection() const { return WorldMoveDirection; }

    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetDesiredWorldLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintPure, Category = "Control|Movement")
    FVector GetDesiredWorldLocation() const { return DesiredWorldLocation; }

    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetDirectionAcceptanceRadius(float Radius);

    UFUNCTION(BlueprintPure, Category = "Control|Movement")
    float GetDirectionAcceptanceRadius() const { return DirectionAcceptanceRadius; }

    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetDirectionControlStrength(float Strength);

    UFUNCTION(BlueprintPure, Category = "Control|Movement")
    float GetDirectionControlStrength() const { return DirectionControlStrength; }

    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetForwardBackIntent(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetLeftRightIntent(float Weight);
    
    UFUNCTION(BlueprintPure, Category = "Control|Movement")
    bool IsSeekingDestination() const { return bSeekingDestination; }

    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void ClearDestination();

    /**
     * Drives WorldMoveDirection toward TargetLocation each time it's
     * called. Zeroes WorldMoveDirection once within DirectionAcceptanceRadius.
     * Intended to be called every tick by whatever is steering the Actor
     * (AI task, script, etc.) — it does not run on its own.
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void MoveToWorldLocation(const FVector& TargetLocation);


    //=========================================================================
    // Individual Intent Setters
    //=========================================================================

    UFUNCTION(BlueprintCallable, Category = "Control|Actions") void SetJumpIntent(float Weight);
    UFUNCTION(BlueprintCallable, Category = "Control|Actions") void SetInteractIntent(float Weight);
    UFUNCTION(BlueprintCallable, Category = "Control|Actions") void SetSprintIntent(float Weight);
    UFUNCTION(BlueprintCallable, Category = "Control|Actions") void SetCrouchIntent(float Weight);
    UFUNCTION(BlueprintCallable, Category = "Control|Actions") void SetDashIntent(float Weight);
    UFUNCTION(BlueprintCallable, Category = "Control|Actions") void SetAimIntent(float Weight);
    UFUNCTION(BlueprintCallable, Category = "Control|Actions") void SetFireIntent(float Weight);
    UFUNCTION(BlueprintCallable, Category = "Control|Actions") void SetReloadIntent(float Weight);


    //=========================================================================
    // Intent Getters
    //=========================================================================

    UFUNCTION(BlueprintPure, Category = "Control|Actions") float GetJumpIntent() const { return JumpIntent; }
    UFUNCTION(BlueprintPure, Category = "Control|Actions") float GetInteractIntent() const { return InteractIntent; }
    UFUNCTION(BlueprintPure, Category = "Control|Actions") float GetSprintIntent() const { return SprintIntent; }
    UFUNCTION(BlueprintPure, Category = "Control|Actions") float GetCrouchIntent() const { return CrouchIntent; }
    UFUNCTION(BlueprintPure, Category = "Control|Actions") float GetDashIntent() const { return DashIntent; }
    UFUNCTION(BlueprintPure, Category = "Control|Actions") float GetAimIntent() const { return AimIntent; }
    UFUNCTION(BlueprintPure, Category = "Control|Actions") float GetFireIntent() const { return FireIntent; }
    UFUNCTION(BlueprintPure, Category = "Control|Actions") float GetReloadIntent() const { return ReloadIntent; }

    /** Gets the weight associated with any intent. */
    UFUNCTION(BlueprintPure, Category = "Control")
    float GetIntentWeight(EControlIntent Intent) const;

    /** Sets the weight associated with any intent. */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void SetIntentWeight(EControlIntent Intent, float Weight);


    //=========================================================================
    // Intent Query Functions
    //=========================================================================

    UFUNCTION(BlueprintPure, Category = "Control")
    bool HasIntent(EControlIntent Intent, float MinimumWeight = 0.01f) const;

    UFUNCTION(BlueprintPure, Category = "Control|Movement")
    bool HasMovementIntent() const;

    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    bool HasActionIntent() const;

    UFUNCTION(BlueprintPure, Category = "Control|Actions") bool WantsToSprint() const;
    UFUNCTION(BlueprintPure, Category = "Control|Actions") bool WantsToCrouch() const;
    UFUNCTION(BlueprintPure, Category = "Control|Actions") bool WantsToDash() const;
    UFUNCTION(BlueprintPure, Category = "Control|Actions") bool WantsToAim() const;
    UFUNCTION(BlueprintPure, Category = "Control|Actions") bool WantsToFire() const;


    //=========================================================================
    // Look Functions
    //=========================================================================

    UFUNCTION(BlueprintCallable, Category = "Control|Look")
    void SetLookInput(const FVector2D& Input);

    UFUNCTION(BlueprintPure, Category = "Control|Look")
    FVector2D GetLookInput() const { return LookInput; }

    UFUNCTION(BlueprintCallable, Category = "Control|Look")
    void SetLookDirection(const FVector& Direction);

    UFUNCTION(BlueprintPure, Category = "Control|Look")
    FVector GetLookDirection() const { return LookDirection; }

    UFUNCTION(BlueprintCallable, Category = "Control|Look")
    void ClearLookInput();


    //=========================================================================
    // Clearing Functions
    //=========================================================================

    /** Clears all control intent (movement, actions, and look). */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void ClearIntents();

    /** Clears MoveIntent and WorldMoveDirection. */
    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void ClearMovementIntent();

    /** Clears all action intents. */
    UFUNCTION(BlueprintCallable, Category = "Control|Actions")
    void ClearActionIntent();


protected:

    virtual void BeginPlay() override;

    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction
    ) override;


    bool bSeekingDestination = false;
};
