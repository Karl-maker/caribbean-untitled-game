#include "CaribbeanAssetSettings.h"

const UCaribbeanAssetSettings& UCaribbeanAssetSettings::Get()
{
    return *GetDefault<UCaribbeanAssetSettings>();
}

const FCharacterVisualProfile* UCaribbeanAssetSettings::FindCharacterVisualProfile(
    FName ProfileKey
) const
{
    return CharacterVisualProfiles.FindByPredicate(
        [ProfileKey](const FCharacterVisualProfile& Profile)
        {
            return Profile.ProfileKey == ProfileKey;
        }
    );
}

const FAnimationProfile* UCaribbeanAssetSettings::FindAnimationProfile(
    FName ProfileKey
) const
{
    return AnimationProfiles.FindByPredicate(
        [ProfileKey](const FAnimationProfile& Profile)
        {
            return Profile.ProfileKey == ProfileKey;
        }
    );
}

const FControlMappingProfile* UCaribbeanAssetSettings::FindControlMappingProfile(
    FName ProfileKey
) const
{
    return ControlMappingProfiles.FindByPredicate(
        [ProfileKey](const FControlMappingProfile& Profile)
        {
            return Profile.ProfileKey == ProfileKey;
        }
    );
}
