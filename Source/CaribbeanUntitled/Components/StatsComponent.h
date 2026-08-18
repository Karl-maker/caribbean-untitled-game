#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatsComponent.generated.h"

/**
 * Stores gameplay attributes and resources for an Actor.
 *
 * StatsComponent owns gameplay values such as:
 * - Movement speeds
 * - Stamina
 * - Dash values
 * - Jump strength
 * - Health
 * - Combat stats
 *
 * Unreal's CharacterMovementComponent owns the actual
 * movement simulation/state.
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
    // MOVEMENT
    // ============================================================

    /**
     * Normal walking speed.
     *
     * This is a gameplay stat.
     * MovementSystem applies it to CharacterMovementComponent.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement",
        meta=(ClampMin="0.0")
    )
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Stats|Movement")
    float JumpAscentGravityScale = 5.5f;
    
    UPROPERTY(EditAnywhere, Category = "Stats|Movement")
    float JumpAnticipationDelay = 0.5f;

    /**
     * Sprinting speed.
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
     * Stamina consumed per second while sprinting.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement|Sprint",
        meta=(ClampMin="0.0")
    )
    float SprintStaminaCostPerSecond = 10.0f;


    /**
     * Jump strength used by the movement system.
     *
     * This is a gameplay stat. The movement system applies
     * it to CharacterMovementComponent::JumpZVelocity.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Movement|Jump",
        meta=(ClampMin="0.0")
    )
    float JumpStrength = 500.0f;


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
    // HEALTH
    // ============================================================

    /**
     * Maximum health this Actor can have.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Health",
        meta=(ClampMin="0.0")
    )
    float MaxHealth = 100.0f;

    /**
     * Current health.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Stats|Health",
        meta=(ClampMin="0.0")
    )
    float Health = 100.0f;

    UFUNCTION(BlueprintPure, Category="Stats|Health")
    float GetHealth() const;

    UFUNCTION(BlueprintPure, Category="Stats|Health")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintPure, Category="Stats|Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category="Stats|Health")
    bool ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category="Stats|Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category="Stats|Health")
    void SetHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category="Stats|Health")
    void RestoreFullHealth();

    UFUNCTION(BlueprintPure, Category="Stats|Health")
    bool IsDead() const;


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

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Sight",
        meta=(ClampMin="0.0")
    )
    float SightRange = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Sight", meta=(ClampMin="0.0", ClampMax="360.0"))
    float SightAngle = 90.0f;
    
    
    // ============================================================
    // COMBAT STATS
    // ============================================================

    /**
     * Base strength affecting melee damage output.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Combat",
        meta=(ClampMin="0.0")
    )
    float Strength = 10.0f;

    /**
     * Base defense reducing incoming damage.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Combat",
        meta=(ClampMin="0.0")
    )
    float Defense = 5.0f;

    /**
     * Poise determines resistance to stagger/interruption.
     * Higher poise = harder to stagger.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Combat",
        meta=(ClampMin="0.0")
    )
    float Poise = 10.0f;


    // ============================================================
    // BLOCK
    // ============================================================

    /**
     * Damage reduction percentage while blocking (0.0 = no reduction, 1.0 = full reduction).
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Block",
        meta=(ClampMin="0.0", ClampMax="1.0")
    )
    float BlockDamageReduction = 0.8f;

    /**
     * Stamina cost per hit blocked.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Block",
        meta=(ClampMin="0.0")
    )
    float BlockStaminaCost = 10.0f;

    /**
     * Angle (in degrees) within which blocks are effective.
     * 180 = full frontal protection, 90 = narrow frontal cone.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Block",
        meta=(ClampMin="0.0", ClampMax="180.0")
    )
    float BlockAngle = 120.0f;


    // ============================================================
    // DODGE
    // ============================================================

    /**
     * Distance travelled during a dodge roll/evasion.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Dodge",
        meta=(ClampMin="0.0")
    )
    float DodgeDistance = 500.0f;

    /**
     * Duration of the dodge animation/movement.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Dodge",
        meta=(ClampMin="0.01")
    )
    float DodgeDuration = 0.3f;

    /**
     * Stamina cost to perform a dodge.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Dodge",
        meta=(ClampMin="0.0")
    )
    float DodgeStaminaCost = 25.0f;

    /**
     * Time before another dodge can be performed.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Dodge",
        meta=(ClampMin="0.0")
    )
    float DodgeCooldown = 0.6f;

    /**
     * Duration of invulnerability frames during a dodge.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category="Stats|Dodge",
        meta=(ClampMin="0.0")
    )
    float DodgeInvulnerabilityDuration = 0.15f;
    
    
    // ============================================================
    // GETTERS
    // ============================================================

    UFUNCTION(BlueprintPure, Category="Stats|Movement")
    float GetWalkSpeed() const;

    UFUNCTION(BlueprintPure, Category="Stats|Movement")
    float GetSprintSpeed() const;

    UFUNCTION(BlueprintPure, Category="Stats|Movement")
    float GetCrouchSpeed() const;

    UFUNCTION(BlueprintPure, Category="Stats|Movement")
    float GetSprintStaminaCostPerSecond() const;

    UFUNCTION(BlueprintPure, Category="Stats|Movement")
    float GetJumpStrength() const;

    UFUNCTION(BlueprintPure, Category="Stats|Movement|Dash")
    float GetDashSpeed() const;

    UFUNCTION(BlueprintPure, Category="Stats|Movement|Dash")
    float GetDashDistance() const;

    UFUNCTION(BlueprintPure, Category="Stats|Movement|Dash")
    float GetDashDuration() const;
    
    UFUNCTION(BlueprintPure, Category="Stats|Sight")
    float GetSightRange() const;

    UFUNCTION(BlueprintPure, Category="Stats|Sight")
    float GetSightAngle() const;

    // ============================================================
    // COMBAT GETTERS
    // ============================================================

    UFUNCTION(BlueprintPure, Category="Stats|Combat")
    float GetStrength() const;

    UFUNCTION(BlueprintPure, Category="Stats|Combat")
    float GetDefense() const;

    UFUNCTION(BlueprintPure, Category="Stats|Combat")
    float GetPoise() const;

    UFUNCTION(BlueprintPure, Category="Stats|Block")
    float GetBlockDamageReduction() const;

    UFUNCTION(BlueprintPure, Category="Stats|Block")
    float GetBlockStaminaCost() const;

    UFUNCTION(BlueprintPure, Category="Stats|Block")
    float GetBlockAngle() const;

    UFUNCTION(BlueprintPure, Category="Stats|Dodge")
    float GetDodgeDistance() const;

    UFUNCTION(BlueprintPure, Category="Stats|Dodge")
    float GetDodgeDuration() const;

    UFUNCTION(BlueprintPure, Category="Stats|Dodge")
    float GetDodgeStaminaCost() const;

    UFUNCTION(BlueprintPure, Category="Stats|Dodge")
    float GetDodgeCooldown() const;

    UFUNCTION(BlueprintPure, Category="Stats|Dodge")
    float GetDodgeInvulnerabilityDuration() const;

protected:

    virtual void BeginPlay() override;
};
