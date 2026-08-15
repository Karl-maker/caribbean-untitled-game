//
// MovementAnimationSystem.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "../Components/AnimationRegistryComponent.h" // EAnimationKey
#include "MovementAnimationSystem.generated.h"

class UAnimationSystem;
class UEventBusSubsystem;
struct FMovementPayload;
enum class EControlIntent : uint8;


/**
 * Translates player/AI movement input (EControlIntent, via
 * FMovementPayload from UControlComponent) into animation requests.
 *
 * This system does NOT play animations, hold per-actor state, or know
 * about transition rules, montages, or the animation registry at all
 * — that's all owned by UAnimationSystem. This system's only job is
 * the seam between "what is the player doing" and "what animation
 * key does that mean", so it stays this small on purpose:
 *
 *     FMovementPayload (EControlIntent)
 *         -> MapControlIntentToAnimationKey
 *             -> UAnimationSystem::RequestAnimationState
 *
 * Anything that isn't expressed as an EControlIntent (combat, hit
 * reactions, death, mounting, cutscenes, ...) should talk to
 * UAnimationSystem directly instead of going through this system.
 *
 * Extending: EControlIntent grows a value that should drive
 * animation -> add a case to MapControlIntentToAnimationKey. No other
 * change needed here; new animation behavior for that key (combos,
 * intermediate animations) is configured on UAnimationSystem's
 * transition rules, not here.
 */
UCLASS()
class CARIBBEANUNTITLED_API UMovementAnimationSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    //=========================================================================
    // USubsystem
    //=========================================================================

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;


    //=========================================================================
    // Configuration
    //=========================================================================

    /**
     * Locomotion key played for MoveForward / MoveBack / MoveLeft /
     * MoveRight control intents. Directional intents don't carry a
     * speed, so they all resolve to the same key by default — swap
     * this for EAnimationKey::Run if that's the right default gait,
     * or replace MapControlIntentToAnimationKey entirely once you
     * have a speed-aware movement payload.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Movement")
    EAnimationKey DefaultLocomotionKey = EAnimationKey::Walk;


private:

    UEventBusSubsystem* GetEventBus() const;
    UAnimationSystem* GetAnimationSystem() const;

    /**
     * Bus handler for the movement/control payload. Each
     * FMovementPayload is a one-shot "this control state was just
     * entered" event (per ControlComponent), so it's translated
     * straight into a RequestAnimationState call — no held/released
     * bookkeeping needed here.
     */
    void OnMovementControlEvent(const FMovementPayload& Payload);

    /**
     * Translates a control intent into an animation key.
     *
     * This is the seam between "what the player is doing" and "what
     * animation that means" — the only place that needs to change
     * when EControlIntent grows new values. Intents with no
     * corresponding EAnimationKey yet (Crouch, Dash, Aim, Fire,
     * Reload as of writing) return false; add the EAnimationKey entry
     * and a case here once those are ready to animate.
     */
    bool MapControlIntentToAnimationKey(EControlIntent Intent, EAnimationKey& OutKey) const;
};
