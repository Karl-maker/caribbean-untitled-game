#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatsComponent.generated.h"

/**
 * Stores gameplay attributes and resources for an Actor.
 *
 * Movement behavior and base movement properties belong to
 * Unreal's CharacterMovementComponent.
 *
 * StatsComponent contains gameplay values that systems can
 * use to modify or drive those systems.
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


    UFUNCTION(BlueprintPure, Category="Stats|Stamina")
    float GetStamina() const;

    UFUNCTION(BlueprintPure, Category="Stats|Stamina")
    float GetMaxStamina() const;

    UFUNCTION(BlueprintPure, Category="Stats|Stamina")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintPure, Category="Stats|Stamina")
    bool HasStamina(float Amount) const;

    UFUNCTION(BlueprintCallable, Category="Stats|Stamina")
    bool ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category="Stats|Stamina")
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category="Stats|Stamina")
    void SetStamina(float NewStamina);

    UFUNCTION(BlueprintCallable, Category="Stats|Stamina")
    void RestoreFullStamina();


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
     * Duration of the dash.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement|Dash",
        meta=(ClampMin="0.01")
    )
    float DashDuration = 0.20f;


    /**
     * Dash speed.
     *
     * This can be calculated from DashDistance / DashDuration,
     * but remains exposed for tuning.
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


    UFUNCTION(BlueprintPure, Category="Stats|Movement|Dash")
    float GetDashSpeed() const;

    UFUNCTION(BlueprintPure, Category="Stats|Movement|Dash")
    float GetDashDistance() const;

    UFUNCTION(BlueprintPure, Category="Stats|Movement|Dash")
    float GetDashDuration() const;


protected:

    virtual void BeginPlay() override;
};
