//
// CombatAnimationSystem.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatAnimationSystem.generated.h"

class UAnimationSystem;
class UEventBusSubsystem;
struct FAttackStartedPayload;

/**
 * Translates combat start events into animation requests.
 *
 * This mirrors UMovementAnimationSystem: combat systems publish a
 * start payload, this subsystem resolves the actor's current combat
 * animation key, and then forwards that key to UAnimationSystem.
 */
UCLASS()
class CARIBBEANUNTITLED_API UCombatAnimationSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    UAnimationSystem* GetAnimationSystem() const;
    UEventBusSubsystem* GetEventBus() const;

    void OnAttackStarted(const FAttackStartedPayload& Payload);
};
