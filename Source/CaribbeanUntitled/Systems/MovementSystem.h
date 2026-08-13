#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MovementSystem.generated.h"

class AGameActor;
class UControlComponent;

/**
 * Converts control intent into movement/motion.
 *
 * Pipeline:
 *
 * ControlComponent
 *      ↓
 * MovementSystem
 *      ↓
 * GameMovementComponent
 *      ↓
 * MotionComponent
 *
 * This system does NOT move Actors directly.
 */
UCLASS()
class CARIBBEANUNTITLED_API UMovementSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    void Tick(float DeltaTime);

protected:

};
