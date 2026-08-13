//
// GameplaySystemScheduler.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "GameplaySystemScheduler.generated.h"

class UMovementSystem;

/**
 * Central scheduler for gameplay systems.
 *
 * The scheduler owns the execution order of gameplay systems.
 *
 * Systems do not tick themselves.
 *
 * Execution order:
 *
 * Input
 *   ↓
 * Gameplay
 *   ↓
 * Movement
 *   ↓
 * Physics
 *   ↓
 * PostPhysics
 *   ↓
 * Transform
 */
UCLASS()
class CARIBBEANUNTITLED_API UGameplaySystemScheduler
    : public UWorldSubsystem
    , public FTickableGameObject
{
    GENERATED_BODY()

public:

    // ------------------------------------------------------------
    // UWorldSubsystem
    // ------------------------------------------------------------

    virtual void Initialize(
        FSubsystemCollectionBase& Collection
    ) override;

    virtual void Deinitialize() override;


    // ------------------------------------------------------------
    // FTickableGameObject
    // ------------------------------------------------------------

    virtual void Tick(float DeltaTime) override;

    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(
            UGameplaySystemScheduler,
            STATGROUP_Tickables
        );
    }

    virtual bool IsTickable() const override
    {
        return true;
    }


private:

    // ------------------------------------------------------------
    // SYSTEMS
    // ------------------------------------------------------------

    UPROPERTY()
    TObjectPtr<UMovementSystem> MovementSystem;



    // ------------------------------------------------------------
    // PHASES
    // ------------------------------------------------------------

    void TickInput(float DeltaTime);

    void TickGameplay(float DeltaTime);

    void TickMovement(float DeltaTime);

    void TickPhysics(float DeltaTime);

    void TickPostPhysics(float DeltaTime);

    void TickTransform(float DeltaTime);
};
