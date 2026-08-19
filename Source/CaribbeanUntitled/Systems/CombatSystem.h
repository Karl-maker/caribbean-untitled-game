#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatSystem.generated.h"

struct FCombatCollisionPayload;
class UEventBusSubsystem;

/**
 * Handles combat gameplay logic after combat collisions
 * have been detected by UCombatDetectionSystem.
 *
 * For now this system only listens for combat collision
 * events and logs them.
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
 *     - Damage
 *     - Hit reactions
 *     - Blocking
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
};
