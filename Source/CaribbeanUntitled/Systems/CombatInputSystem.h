#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tickable.h"
#include "CombatInputSystem.generated.h"

class UActorRegistry;
class UControlComponent;
class UCombatComponent;

/**
 * Translates control/input intent into combat actions.
 *
 * Architecture:
 *
 *     PlayerInputComponent
 *             |
 *             v
 *     UControlComponent
 *             |
 *             v
 *     UCombatInputSystem
 *             |
 *             v
 *     UCombatComponent
 *             |
 *             v
 *        CombatSystem
 *
 * The system does not perform combat itself.
 * It only converts player intent into combat requests.
 */
UCLASS()
class CARIBBEANUNTITLED_API UCombatInputSystem : public UObject
{
    GENERATED_BODY()

public:

    //=========================================================================
    // Tick
    //=========================================================================

    virtual void Tick(float DeltaTime);

    virtual TStatId GetStatId() const;

    virtual bool IsTickable() const
    {
        return true;
    }

    //=========================================================================
    // Cleanup
    //=========================================================================

    /**
     * Cleans up cached input state.
     */
    void Cleanup();

private:

    //=========================================================================
    // Processing
    //=========================================================================

    /**
     * Processes combat input for a single actor.
     */
    void ProcessActor(AActor* Actor);

    //=========================================================================
    // Actor Registry
    //=========================================================================

    /**
     * Gets the world's ActorRegistry subsystem.
     */
    UActorRegistry* GetActorRegistry() const;

    //=========================================================================
    // Input State
    //=========================================================================

    /**
     * Tracks whether each combat input was pressed during the previous tick.
     *
     * This lets us detect a rising edge so we only request an attack once
     * when the button is pressed, instead of every tick while held.
     */
    TMap<TWeakObjectPtr<UControlComponent>, bool> PreviousLightAttackState;
    TMap<TWeakObjectPtr<UControlComponent>, bool> PreviousHeavyAttackState;
};
