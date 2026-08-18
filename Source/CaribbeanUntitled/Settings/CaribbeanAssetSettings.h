#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "../Components/AnimationRegistryComponent.h"
#include "CaribbeanAssetSettings.generated.h"

class UAnimInstance;
class UAnimSequence;
class UCurveFloat;
class UInputAction;
class UInputMappingContext;
class USkeletalMesh;

USTRUCT(BlueprintType)
struct FCharacterVisualProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FName ProfileKey = NAME_None;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath SkeletalMesh;

    UPROPERTY(EditAnywhere)
    FSoftClassPath AnimBlueprintClass;
};

USTRUCT(BlueprintType)
struct FAnimationAssetEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    EAnimationKey Key = EAnimationKey::None;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath Animation;

    UPROPERTY(EditAnywhere)
    EAnimationBlendMode BlendMode = EAnimationBlendMode::FullBody;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath BlendCurve;
};

USTRUCT(BlueprintType)
struct FAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FName ProfileKey = NAME_None;

    UPROPERTY(EditAnywhere)
    TArray<FAnimationAssetEntry> Entries;
};

USTRUCT(BlueprintType)
struct FControlMappingProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FName ProfileKey = NAME_None;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath DefaultMappingContext;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath MoveForwardAction;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath MoveBackwardAction;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath MoveLeftAction;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath MoveRightAction;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath LookAction;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath JumpAction;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath SprintAction;

    UPROPERTY(EditAnywhere)
    FSoftObjectPath CrouchAction;
};

UCLASS(Config=Game, DefaultConfig)
class CARIBBEANUNTITLED_API UCaribbeanAssetSettings : public UObject
{
    GENERATED_BODY()

public:

    static const UCaribbeanAssetSettings& Get();

    const FCharacterVisualProfile* FindCharacterVisualProfile(FName ProfileKey) const;
    const FAnimationProfile* FindAnimationProfile(FName ProfileKey) const;
    const FControlMappingProfile* FindControlMappingProfile(FName ProfileKey) const;

private:

    UPROPERTY(Config, EditAnywhere, Category="Assets")
    TArray<FCharacterVisualProfile> CharacterVisualProfiles;

    UPROPERTY(Config, EditAnywhere, Category="Assets")
    TArray<FAnimationProfile> AnimationProfiles;

    UPROPERTY(Config, EditAnywhere, Category="Assets")
    TArray<FControlMappingProfile> ControlMappingProfiles;
};
