#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "../Components/AnimationRegistryComponent.h"
#include "../Components/CombatVolumeComponent.h"
#include "CaribbeanAssetSettings.generated.h"

class UAnimInstance;
class UAnimSequence;
class UInputAction;
class UInputMappingContext;
class USkeletalMesh;

USTRUCT(BlueprintType)
struct FCharacterVisualProfile
{
    GENERATED_BODY()

    /**
     * Stable key used by characters to pick a visual setup.
     *
     * Example:
     *   DEMO_FEMALE
     */
    UPROPERTY(EditAnywhere)
    FName ProfileKey = NAME_None;

    /**
     * Skeletal mesh used by this visual profile.
     */
    UPROPERTY(EditAnywhere)
    FSoftObjectPath SkeletalMesh;

    /**
     * Animation blueprint class used by this visual profile.
     */
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
};

USTRUCT(BlueprintType)
struct FAnimationProfile
{
    GENERATED_BODY()

    /**
     * Stable key used by characters to pick animation assets.
     *
     * Example:
     *   HUMANOID_ANIMATIONS
     */
    UPROPERTY(EditAnywhere)
    FName ProfileKey = NAME_None;

    /**
     * Animation entries keyed by gameplay animation enum.
     */
    UPROPERTY(EditAnywhere)
    TArray<FAnimationAssetEntry> Entries;
};

USTRUCT(BlueprintType)
struct FCombatAttachmentProfile
{
    GENERATED_BODY()

    /**
     * Profile key used by characters to resolve this attachment map.
     *
     * Example:
     *   DEMO_FEMALE
     */
    UPROPERTY(EditAnywhere)
    FName ProfileKey = NAME_None;

    /**
     * Bone names for each combat volume body part.
     *
     * This lets you swap skeletons later without changing code.
     */
    UPROPERTY(EditAnywhere, Category="Combat Attachments")
    TMap<EBodyPart, FName> BoneNamesByBodyPart;
};

USTRUCT(BlueprintType)
struct FControlMappingProfile
{
    GENERATED_BODY()

    /**
     * Stable key used by characters to pick input mappings.
     *
     * Example:
     *   CONTROL_MAPPINGS_DESKTOP
     */
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

    UPROPERTY(EditAnywhere)
    FSoftObjectPath LightAttackAction;
};

UCLASS(Config=Game, DefaultConfig)
class CARIBBEANUNTITLED_API UCaribbeanAssetSettings : public UObject
{
    GENERATED_BODY()

public:

    static const UCaribbeanAssetSettings& Get();

    const FCharacterVisualProfile* FindCharacterVisualProfile(FName ProfileKey) const;
    const FAnimationProfile* FindAnimationProfile(FName ProfileKey) const;
    const FCombatAttachmentProfile* FindCombatAttachmentProfile(FName ProfileKey) const;
    const FControlMappingProfile* FindControlMappingProfile(FName ProfileKey) const;

private:

    /**
     * Add one entry per visual variant, animation set, combat attachment map,
     * and control mapping set.
     *
     * Recommended workflow:
     * - Create a matching ProfileKey across the relevant arrays.
     * - Point the visual profile at the skeletal mesh and anim blueprint.
     * - Point the animation profile at the animation assets for that rig.
     * - Point the combat attachment profile at the rig's bone names.
     * - Keep the same ProfileKey on the character so all lookups stay in sync.
     */
    UPROPERTY(Config, EditAnywhere, Category="Assets")
    TArray<FCharacterVisualProfile> CharacterVisualProfiles;

    UPROPERTY(Config, EditAnywhere, Category="Assets")
    TArray<FAnimationProfile> AnimationProfiles;

    UPROPERTY(Config, EditAnywhere, Category="Assets")
    TArray<FCombatAttachmentProfile> CombatAttachmentProfiles;

    UPROPERTY(Config, EditAnywhere, Category="Assets")
    TArray<FControlMappingProfile> ControlMappingProfiles;
};
