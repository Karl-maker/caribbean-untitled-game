//
//  MovementComponent.h
//  CaribbeanUntitled
//
//  Created by Karl-Johan Bailey on 8/12/26.
//

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMovementComponent.generated.h"


/**
 * Current movement mode.
 */
UENUM(BlueprintType)
enum class EGameMovementMode : uint8
{
    Grounded,
    Airborne,
    Crouching,
    Sprinting,
    Dashing,
    Swimming
};


/**
 * Stores movement configuration and runtime movement state.
 *
 * UControlComponent represents what the Actor wants to do.
 *
 * UMovementComponent represents the movement state and the
 * desired movement produced by gameplay systems.
 *
 * The component does not directly move the Actor.
 */
UCLASS(
    ClassGroup=(Gameplay),
    BlueprintType,
    Blueprintable,
    meta=(BlueprintSpawnableComponent)
)
class CARIBBEANUNTITLED_API UGameMovementComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UGameMovementComponent();


    // ============================================================
    // MOVEMENT SPEED
    // ============================================================

    /**
     * Normal walking speed.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Speed",
        meta=(ClampMin="0.0")
    )
    float WalkSpeed = 300.0f;


    /**
     * Sprinting speed.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Speed",
        meta=(ClampMin="0.0")
    )
    float SprintSpeed = 600.0f;


    /**
     * Crouching speed.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Speed",
        meta=(ClampMin="0.0")
    )
    float CrouchSpeed = 150.0f;


    /**
     * Dash movement speed.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Dash",
        meta=(ClampMin="0.0")
    )
    float DashSpeed = 1200.0f;


    // ============================================================
    // ACCELERATION
    // ============================================================

    /**
     * Ground acceleration.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Acceleration",
        meta=(ClampMin="0.0")
    )
    float Acceleration = 2000.0f;


    /**
     * Deceleration when movement intent decreases.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Acceleration",
        meta=(ClampMin="0.0")
    )
    float Deceleration = 2500.0f;


    /**
     * Acceleration while airborne.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Acceleration",
        meta=(ClampMin="0.0")
    )
    float AirAcceleration = 800.0f;


    // ============================================================
    // JUMP
    // ============================================================

    /**
     * Initial vertical velocity applied when jumping.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Jump",
        meta=(ClampMin="0.0")
    )
    float JumpVelocity = 600.0f;


    /**
     * Whether this movement configuration supports jumping.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Jump"
    )
    bool bCanJump = true;


    // ============================================================
    // SPRINT
    // ============================================================

    /**
     * Stamina consumed per second while sprinting.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Sprint",
        meta=(ClampMin="0.0")
    )
    float SprintStaminaCost = 10.0f;


    // ============================================================
    // DASH
    // ============================================================

    /**
     * Duration of a dash in seconds.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Dash",
        meta=(ClampMin="0.0")
    )
    float DashDuration = 0.2f;


    /**
     * Cooldown between dashes.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Dash",
        meta=(ClampMin="0.0")
    )
    float DashCooldown = 1.0f;


    /**
     * Stamina required to perform a dash.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Movement|Dash",
        meta=(ClampMin="0.0")
    )
    float DashStaminaCost = 25.0f;


    // ============================================================
    // RUNTIME MOVEMENT STATE
    // ============================================================

    /**
     * Direction the Actor currently wants to move.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Movement|State"
    )
    FVector DesiredDirection = FVector::ZeroVector;

    /**
     * Current movement mode.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Movement|State"
    )
    EGameMovementMode MovementMode = EGameMovementMode::Grounded;


    /**
     * Whether the Actor is currently grounded.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Movement|State"
    )
    bool bIsGrounded = true;


    /**
     * Whether the Actor is currently sprinting.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Movement|State"
    )
    bool bIsSprinting = false;


    /**
     * Whether the Actor is currently crouching.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Movement|State"
    )
    bool bIsCrouching = false;


    /**
     * Whether the Actor is currently dashing.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Movement|State"
    )
    bool bIsDashing = false;


    /**
     * Remaining dash time.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Movement|Dash"
    )
    float DashTimeRemaining = 0.0f;


    /**
     * Remaining dash cooldown.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Movement|Dash"
    )
    float DashCooldownRemaining = 0.0f;


    // ============================================================
    // ACCESSORS
    // ============================================================

    UFUNCTION(BlueprintPure, Category="Movement")
    FVector GetDesiredDirection() const;


    UFUNCTION(BlueprintPure, Category="Movement")
    EGameMovementMode GetMovementMode() const;


    UFUNCTION(BlueprintPure, Category="Movement")
    bool IsGrounded() const;


    UFUNCTION(BlueprintPure, Category="Movement")
    bool IsSprinting() const;


    UFUNCTION(BlueprintPure, Category="Movement")
    bool IsCrouching() const;


    UFUNCTION(BlueprintPure, Category="Movement")
    bool IsDashing() const;


    // ============================================================
    // MOVEMENT STATE
    // ============================================================

    UFUNCTION(BlueprintCallable, Category="Movement")
    void SetDesiredDirection(const FVector& Direction);

    UFUNCTION(BlueprintCallable, Category="Movement")
    void SetMovementMode(EGameMovementMode NewMode);


    UFUNCTION(BlueprintCallable, Category="Movement")
    void SetGrounded(bool bGrounded);


    UFUNCTION(BlueprintCallable, Category="Movement")
    void ClearDesiredMovement();


    // ============================================================
    // MOVEMENT STATES
    // ============================================================

    UFUNCTION(BlueprintCallable, Category="Movement")
    void SetSprinting(bool bSprinting);


    UFUNCTION(BlueprintCallable, Category="Movement")
    void SetCrouching(bool bCrouching);


    UFUNCTION(BlueprintCallable, Category="Movement")
    void SetDashing(bool bDashing);


    // ============================================================
    // DASH
    // ============================================================

    UFUNCTION(BlueprintPure, Category="Movement|Dash")
    bool CanDash() const;


    UFUNCTION(BlueprintCallable, Category="Movement|Dash")
    void StartDash();


    UFUNCTION(BlueprintCallable, Category="Movement|Dash")
    void StopDash();


    UFUNCTION(BlueprintCallable, Category="Movement|Dash")
    void SetDashCooldown(float RemainingTime);


    // ============================================================
    // UTILITY
    // ============================================================

    UFUNCTION(BlueprintPure, Category="Movement")
    float GetCurrentMaxSpeed() const;


protected:

    virtual void BeginPlay() override;
};
