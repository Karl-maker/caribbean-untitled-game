#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatSystem.generated.h"

struct FCombatCollisionPayload;
class AActor;
class UEventBusSubsystem;

USTRUCT(BlueprintType)
struct CARIBBEANUNTITLED_API FCombatResolvedPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    TObjectPtr<AActor> SourceActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    float RawDamage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    float AppliedDamage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    bool bBlocked = false;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    bool bKilled = false;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    FVector HitLocation = FVector::ZeroVector;
};

/**
 * Handles combat gameplay logic after combat collisions
 * have been detected by UCombatDetectionSystem.
 *
 * Resolves authoritative combat collisions into damage, directional blocks,
 * stamina costs, death state, and a presentation-friendly result event.
 *
 * Architecture:
 *
 *     CombatDetectionSystem
 *             |
 *             v
 *     FCombatCollisionPayload
 *             |
 *             v
 *        EventBusSubsystem
 *             |
 *             v
 *        CombatSystem
 *
 * Future responsibilities can include:
 *
 *     - Hit reactions
 *     - Parrying
 *     - Invulnerability
 *     - Knockback
 *     - Hit stun
 *     - Combat state
 *     - Hit confirmation
 */
UCLASS()
class CARIBBEANUNTITLED_API UCombatSystem
    : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    //=========================================================================
    // Lifecycle
    //=========================================================================

    virtual void Initialize(
        FSubsystemCollectionBase& Collection
    ) override;

    virtual void Deinitialize() override;

private:

    //=========================================================================
    // Event Bus
    //=========================================================================

    /**
     * Returns the world's EventBus subsystem.
     */
    UEventBusSubsystem* GetEventBus() const;

    /**
     * Subscribes to combat events.
     */
    void SubscribeToEvents();

    /**
     * Removes all EventBus subscriptions owned by this system.
     */
    void UnsubscribeFromEvents();

    //=========================================================================
    // Combat Events
    //=========================================================================

    /**
     * Called whenever CombatDetectionSystem detects
     * a combat volume collision.
     */
    void OnCombatCollision(
        const FCombatCollisionPayload& Payload
    );

    bool IsBlockEffective(
        AActor* SourceActor,
        AActor* TargetActor
    ) const;
};
