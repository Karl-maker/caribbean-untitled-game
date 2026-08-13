//
//  StatsComponent.h
//  CaribbeanUntitled
//
//  Created by Karl-Johan Bailey on 8/12/26.
//

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatsComponent.generated.h"


/**
 * Stores general gameplay attributes and resources for an Actor.
 *
 * This component owns runtime gameplay values such as stamina,
 * health, mana, and movement-related attributes.
 *
 * Movement behavior belongs in the Actor's movement component.
 * This component provides the values that movement systems use.
 */
UCLASS(
    ClassGroup=(Gameplay),
    BlueprintType,
    Blueprintable,
    meta=(BlueprintSpawnableComponent)
)
class CARIBBEANUNTITLED_API UStatsComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UStatsComponent();


    // ============================================================
    // STAMINA
    // ============================================================

    /**
     * Maximum amount of stamina this Actor can have.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Stamina",
        meta=(ClampMin="0.0")
    )
    float MaxStamina = 100.0f;


    /**
     * Current stamina.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Stats|Stamina",
        meta=(ClampMin="0.0")
    )
    float Stamina = 100.0f;


    /**
     * Amount of stamina regenerated per second.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Stamina",
        meta=(ClampMin="0.0")
    )
    float StaminaRegenRate = 15.0f;


    /**
     * Time after stamina usage before regeneration begins.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Stamina",
        meta=(ClampMin="0.0")
    )
    float StaminaRegenDelay = 1.0f;


    /**
     * Returns the current stamina.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Stamina")
    float GetStamina() const;


    /**
     * Returns maximum stamina.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Stamina")
    float GetMaxStamina() const;


    /**
     * Returns stamina as a normalized value from 0 to 1.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Stamina")
    float GetStaminaPercent() const;


    /**
     * Returns whether enough stamina is available.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Stamina")
    bool HasStamina(float Amount) const;


    /**
     * Consumes stamina.
     *
     * Returns false if insufficient stamina exists.
     */
    UFUNCTION(BlueprintCallable, Category="Stats|Stamina")
    bool ConsumeStamina(float Amount);


    /**
     * Restores stamina.
     */
    UFUNCTION(BlueprintCallable, Category="Stats|Stamina")
    void RestoreStamina(float Amount);


    /**
     * Sets current stamina directly.
     */
    UFUNCTION(BlueprintCallable, Category="Stats|Stamina")
    void SetStamina(float NewStamina);


    /**
     * Completely restores stamina.
     */
    UFUNCTION(BlueprintCallable, Category="Stats|Stamina")
    void RestoreFullStamina();


    // ============================================================
    // MOVEMENT
    // ============================================================

    /**
     * Normal walking movement speed.
     *
     * Unreal movement speeds are measured in centimeters per second.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement",
        meta=(ClampMin="0.0")
    )
    float WalkSpeed = 300.0f;


    /**
     * Running movement speed.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement",
        meta=(ClampMin="0.0")
    )
    float RunSpeed = 500.0f;


    /**
     * Sprinting movement speed.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement",
        meta=(ClampMin="0.0")
    )
    float SprintSpeed = 700.0f;


    /**
     * Crouching movement speed.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement",
        meta=(ClampMin="0.0")
    )
    float CrouchSpeed = 150.0f;


    /**
     * Maximum speed while airborne.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement",
        meta=(ClampMin="0.0")
    )
    float AirSpeed = 300.0f;


    // ============================================================
    // DASH
    // ============================================================

    /**
     * Distance travelled during a dash.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement|Dash",
        meta=(ClampMin="0.0")
    )
    float DashDistance = 600.0f;


    /**
     * Duration of the dash in seconds.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement|Dash",
        meta=(ClampMin="0.01")
    )
    float DashDuration = 0.20f;


    /**
     * Dash speed in centimeters per second.
     *
     * Usually calculated from DashDistance / DashDuration,
     * but exposed separately so it can be tuned directly.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement|Dash",
        meta=(ClampMin="0.0")
    )
    float DashSpeed = 3000.0f;


    /**
     * Stamina consumed when performing a dash.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement|Dash",
        meta=(ClampMin="0.0")
    )
    float DashStaminaCost = 20.0f;


    /**
     * Time before another dash can be performed.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement|Dash",
        meta=(ClampMin="0.0")
    )
    float DashCooldown = 0.5f;


    // ============================================================
    // MOVEMENT HELPERS
    // ============================================================

    /**
     * Returns the normal walking speed.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Movement")
    float GetWalkSpeed() const;


    /**
     * Returns running speed.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Movement")
    float GetRunSpeed() const;


    /**
     * Returns sprinting speed.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Movement")
    float GetSprintSpeed() const;


    /**
     * Returns crouching speed.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Movement")
    float GetCrouchSpeed() const;


    /**
     * Returns dash speed.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Movement|Dash")
    float GetDashSpeed() const;


    /**
     * Returns dash distance.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Movement|Dash")
    float GetDashDistance() const;


    /**
     * Returns dash duration.
     */
    UFUNCTION(BlueprintPure, Category="Stats|Movement|Dash")
    float GetDashDuration() const;


protected:

    virtual void BeginPlay() override;
};
