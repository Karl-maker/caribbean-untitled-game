#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Enums/Locomotion.h"
#include "MovementPayload.generated.h"

USTRUCT(BlueprintType)
struct FMovementPayload
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<AActor> Source = nullptr;

    UPROPERTY()
    ELocomotion Type = ELocomotion::Stand;
};
