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
 * This component owns runtime values such as stamina, health, mana,
 * and other actor-wide resources.
 *
 * Movement-specific configuration belongs in UMovementComponent.
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


protected:

    virtual void BeginPlay() override;
};
