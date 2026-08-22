//
// IKEventPayload.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "../Components/IKComponent.h"
#include "IKEventPayload.generated.h"

UENUM(BlueprintType)
enum class EIKMode : uint8
{
    Set,
    Clear
};

USTRUCT(BlueprintType)
struct CARIBBEANUNTITLED_API FIKEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    TObjectPtr<AActor> Source = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FName Channel = NAME_None;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    EIKMode Mode = EIKMode::Set;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FIKTargetData Target;
};
