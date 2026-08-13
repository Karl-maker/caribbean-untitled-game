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
 * For third-person control, movement is relative to the actor's
 * current facing direction. The MoveIntent values (-1 to 1) represent
 * the desired movement in local space:
 *   - X: Left (-1) to Right (+1)
 *   - Y: Back (-1) to Forward (+1)
 *
 * The component does not perform movement, jumping, interaction,
 * or other gameplay logic. It only represents what the Actor intends to do.
 *
 * Example usage:
 *   Player input: MoveIntent = (0.0, 1.0)  → Move forward
 *   Player input: MoveIntent = (-1.0, 0.5) → Move left-forward
 *   AI input: MoveIntent = (0.0, 1.0)      → Move toward target
 */
UCLASS(ClassGroup=(Gameplay), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UControlComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UControlComponent();

    //=========================================================================
    // Core Control Properties
    //=========================================================================

    /**
     * Determines the source of this control.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
    EControlType ControlType = EControlType::Player;

    /**
     * Movement intent in LOCAL space relative to the actor's facing direction.
     *
     * X (Horizontal):
     *   -1.0 = Move Left
     *    0.0 = No horizontal movement
     *   +1.0 = Move Right
     *
     * Y (Vertical/Forward):
     *   -1.0 = Move Backward
     *    0.0 = No forward/backward movement
     *   +1.0 = Move Forward
     *
     * Values are normalized and should be clamped to [-1, 1].
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Movement",
        meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    FVector2D MoveIntent = FVector2D::ZeroVector;

    /**
     * Raw world-space movement direction.
     *
     * This is the direction in world space that the actor wants to move.
     * Automatically calculated from MoveIntent relative to actor rotation.
     * Can also be set directly for AI/scripted movement.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Movement")
    FVector WorldMoveDirection = FVector::ZeroVector;

    /**
     * Desired world-space look direction.
     *
     * Represents the direction the controlled Actor wants to face.
     * For player control, this is typically derived from camera orientation.
     * For AI, this is typically the direction toward the target.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Look")
    FVector LookDirection = FVector::ForwardVector;

    /**
     * Look input from mouse/controller.
     *
     * X = Horizontal look input (Yaw)
     * Y = Vertical look input (Pitch)
     *
     * This is raw input, not a world-space direction.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Look")
    FVector2D LookInput = FVector2D::ZeroVector;

    //=========================================================================
    // Action Intents (0.0 = None, 1.0 = Full Intent)
    //=========================================================================

    /**
     * Jump intent.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float JumpIntent = 0.0f;

    /**
     * Interaction intent.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InteractIntent = 0.0f;

    /**
     * Sprint intent.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SprintIntent = 0.0f;

    /**
     * Crouch intent.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CrouchIntent = 0.0f;

    /**
     * Dash intent (request, not guaranteed execution).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DashIntent = 0.0f;

    /**
     * Aim intent.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AimIntent = 0.0f;

    /**
     * Fire/Attack intent.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FireIntent = 0.0f;

    /**
     * Reload intent.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control|Actions",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ReloadIntent = 0.0f;

    //=========================================================================
    // Movement Direction Setters
    //=========================================================================

    /**
     * Sets movement intent in local space.
     *
     * @param Intent - Local space movement where X = Left/Right, Y = Back/Forward
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetMoveIntent(const FVector2D& Intent);

    /**
     * Sets movement intent from a world-space direction.
     * Automatically converts to local space based on actor rotation.
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetWorldMoveDirection(const FVector& WorldDirection);

    /**
     * Gets the world-space movement direction.
     * Calculated from MoveIntent relative to actor rotation.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Movement")
    FVector GetWorldMoveDirection() const;

    /**
     * Sets forward/backward movement weight.
     *
     * @param Weight - -1.0 (Back) to 1.0 (Forward)
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetForwardBackIntent(float Weight);

    /**
     * Sets left/right movement weight.
     *
     * @param Weight - -1.0 (Left) to 1.0 (Right)
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void SetLeftRightIntent(float Weight);

    //=========================================================================
    // Individual Intent Setters
    //=========================================================================

    UFUNCTION(BlueprintCallable, Category = "Control|Actions")
    void SetJumpIntent(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Control|Actions")
    void SetInteractIntent(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Control|Actions")
    void SetSprintIntent(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Control|Actions")
    void SetCrouchIntent(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Control|Actions")
    void SetDashIntent(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Control|Actions")
    void SetAimIntent(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Control|Actions")
    void SetFireIntent(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Control|Actions")
    void SetReloadIntent(float Weight);

    //=========================================================================
    // Intent Getters
    //=========================================================================

    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    float GetJumpIntent() const { return JumpIntent; }

    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    float GetInteractIntent() const { return InteractIntent; }

    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    float GetSprintIntent() const { return SprintIntent; }

    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    float GetCrouchIntent() const { return CrouchIntent; }

    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    float GetDashIntent() const { return DashIntent; }

    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    float GetAimIntent() const { return AimIntent; }

    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    float GetFireIntent() const { return FireIntent; }

    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    float GetReloadIntent() const { return ReloadIntent; }

    /**
     * Gets the weight associated with any intent.
     */
    UFUNCTION(BlueprintPure, Category = "Control")
    float GetIntentWeight(EControlIntent Intent) const;

    /**
     * Sets the weight associated with any intent.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void SetIntentWeight(EControlIntent Intent, float Weight);

    //=========================================================================
    // Intent Query Functions
    //=========================================================================

    /**
     * Returns true if the specified intent is active.
     */
    UFUNCTION(BlueprintPure, Category = "Control")
    bool HasIntent(EControlIntent Intent, float MinimumWeight = 0.01f) const;

    /**
     * Returns true if the actor intends to move in any direction.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Movement")
    bool HasMovementIntent() const;

    /**
     * Returns true if the actor intends to perform any action.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    bool HasActionIntent() const;

    /**
     * Returns true if the actor intends to sprint.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    bool WantsToSprint() const;

    /**
     * Returns true if the actor intends to crouch.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    bool WantsToCrouch() const;

    /**
     * Returns true if the actor intends to dash.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    bool WantsToDash() const;

    /**
     * Returns true if the actor intends to aim.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    bool WantsToAim() const;

    /**
     * Returns true if the actor intends to fire.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Actions")
    bool WantsToFire() const;

    //=========================================================================
    // Look Functions
    //=========================================================================

    /**
     * Sets mouse/controller look input.
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Look")
    void SetLookInput(const FVector2D& Input);

    /**
     * Gets current look input.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Look")
    FVector2D GetLookInput() const { return LookInput; }

    /**
     * Sets the desired world-space look direction.
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Look")
    void SetLookDirection(const FVector& Direction);

    /**
     * Gets the desired look direction.
     */
    UFUNCTION(BlueprintPure, Category = "Control|Look")
    FVector GetLookDirection() const { return LookDirection; }

    /**
     * Clears look input.
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Look")
    void ClearLookInput();

    //=========================================================================
    // Clearing Functions
    //=========================================================================

    /**
     * Clears all control intent.
     * Useful when a system consumes input each frame.
     */
    UFUNCTION(BlueprintCallable, Category = "Control")
    void ClearIntents();

    /**
     * Clears only movement intent.
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Movement")
    void ClearMovementIntent();

    /**
     * Clears only action intent.
     */
    UFUNCTION(BlueprintCallable, Category = "Control|Actions")
    void ClearActionIntent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    /** Updates WorldMoveDirection from MoveIntent and actor rotation. */
    void UpdateWorldMoveDirection();

    /** Cached actor rotation for world-space calculations. */
    FRotator CachedActorRotation;
};
