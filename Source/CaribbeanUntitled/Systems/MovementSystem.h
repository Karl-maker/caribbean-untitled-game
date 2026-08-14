#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MovementSystem.generated.h"

class AActor;

UCLASS()
class CARIBBEANUNTITLED_API UMovementSystem : public UObject
{
    GENERATED_BODY()

public:

    void Tick(float DeltaTime);

private:

    void ProcessActor(
        AActor* Actor,
        float DeltaTime
    );
};
