#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Components/ControlComponent.h"
#include "MovementPayload.generated.h"

USTRUCT(BlueprintType)
struct FMovementPayload
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<AActor> Source = nullptr;

    UPROPERTY()
    EControlIntent Type = EControlIntent::MoveForward;
};
