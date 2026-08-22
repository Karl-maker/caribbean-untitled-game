//
// IKComponent.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "IKComponent.generated.h"

USTRUCT(BlueprintType)
struct CARIBBEANUNTITLED_API FIKTargetData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    bool bEnabled = false;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FVector TargetDirection = FVector::ForwardVector;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    float Weight = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    float BlendSpeed = 4.0f;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FName BoneName = NAME_None;
};

USTRUCT(BlueprintType)
struct CARIBBEANUNTITLED_API FIKTargetEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FName Channel = NAME_None;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FIKTargetData Target;
};

UCLASS(ClassGroup=(Gameplay), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UIKComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UIKComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "IK")
    TArray<FIKTargetEntry> Targets;

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetTarget(FName Channel, const FIKTargetData& Target);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void ClearTarget(FName Channel);

    UFUNCTION(BlueprintPure, Category = "IK")
    bool GetTarget(FName Channel, FIKTargetData& OutTarget) const;

    const FIKTargetData* FindTarget(FName Channel) const;

    UFUNCTION(BlueprintPure, Category = "IK")
    bool HasTarget(FName Channel) const;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    int32 FindTargetIndex(FName Channel) const;
};
