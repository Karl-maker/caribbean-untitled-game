//
// PlayableActor.cpp
//

#include "PlayableActor.h"

#include "../Components/AnimationRegistryComponent.h"
#include "../Components/PlayerInputComponent.h"
#include "../Settings/CaribbeanAssetSettings.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "InputAction.h"
#include "InputMappingContext.h"

namespace
{
    template<typename TObjectType>
    TObjectType* LoadAssetOrNull(const FSoftObjectPath& AssetPath)
    {
        return AssetPath.IsValid()
            ? Cast<TObjectType>(AssetPath.TryLoad())
            : nullptr;
    }

    void ConfigureAnimationDefinition(
        UAnimationRegistryComponent* Registry,
        EAnimationKey Key,
        UAnimSequence* Animation
    )
    {
        if (!Registry)
        {
            return;
        }

        for (FAnimationDefinition& Definition : Registry->Animations)
        {
            if (Definition.Key != Key)
            {
                continue;
            }

            Definition.Animation = Animation;
            return;
        }
    }
}

APlayableActor::APlayableActor()
{
    const UCaribbeanAssetSettings& AssetSettings = UCaribbeanAssetSettings::Get();
    
    // ============================================================
    // PLAYER INPUT
    // ============================================================

    PlayerInputComponent =
        CreateDefaultSubobject<UPlayerInputComponent>(
            TEXT("PlayerInputComponent")
        );


    // ============================================================
    // THIRD-PERSON CAMERA
    // ============================================================

    CameraBoom =
        CreateDefaultSubobject<USpringArmComponent>(
            TEXT("CameraBoom")
        );

    CameraBoom->SetupAttachment(RootComponent);

    CameraBoom->TargetArmLength = 1000.0f;

    // Mouse/controller rotates the camera boom.
    CameraBoom->bUsePawnControlRotation = false;

    // Camera moves closer when blocked by geometry.
    CameraBoom->bDoCollisionTest = false;


    FollowCamera =
        CreateDefaultSubobject<UCameraComponent>(
            TEXT("FollowCamera")
        );

    FollowCamera->SetupAttachment(
        CameraBoom,
        USpringArmComponent::SocketName
    );

    // The boom handles rotation.
    FollowCamera->bUsePawnControlRotation = false;

    if (PlayerInputComponent)
    {
        if (const FControlMappingProfile* ControlProfile =
            AssetSettings.FindControlMappingProfile(
                ControlMappingProfileKey
            ))
        {
            PlayerInputComponent->DefaultMappingContext =
                LoadAssetOrNull<UInputMappingContext>(
                    ControlProfile->DefaultMappingContext
                );

            PlayerInputComponent->MoveForwardAction =
                LoadAssetOrNull<UInputAction>(
                    ControlProfile->MoveForwardAction
                );

            PlayerInputComponent->MoveBackwardAction =
                LoadAssetOrNull<UInputAction>(
                    ControlProfile->MoveBackwardAction
                );

            PlayerInputComponent->MoveLeftAction =
                LoadAssetOrNull<UInputAction>(
                    ControlProfile->MoveLeftAction
                );

            PlayerInputComponent->MoveRightAction =
                LoadAssetOrNull<UInputAction>(
                    ControlProfile->MoveRightAction
                );

            PlayerInputComponent->LookAction =
                LoadAssetOrNull<UInputAction>(
                    ControlProfile->LookAction
                );

            PlayerInputComponent->JumpAction =
                LoadAssetOrNull<UInputAction>(
                    ControlProfile->JumpAction
                );

            PlayerInputComponent->SprintAction =
                LoadAssetOrNull<UInputAction>(
                    ControlProfile->SprintAction
                );

            PlayerInputComponent->CrouchAction =
                LoadAssetOrNull<UInputAction>(
                    ControlProfile->CrouchAction
                );

            PlayerInputComponent->LightAttackAction =
                LoadAssetOrNull<UInputAction>(
                    ControlProfile->LightAttackAction
                );
        }
    }

    if (AnimationRegistryComponent)
    {
        if (const FAnimationProfile* AnimationProfile =
            AssetSettings.FindAnimationProfile(
                AnimationProfileKey
            ))
        {
            for (const FAnimationAssetEntry& Entry :
                AnimationProfile->Entries)
            {
                ConfigureAnimationDefinition(
                    AnimationRegistryComponent,
                    Entry.Key,
                    LoadAssetOrNull<UAnimSequence>(
                        Entry.Animation
                    )
                );
            }
        }
    }

    if (AnimationRegistryComponent)
    {
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::LeftStrafe,
            LoadAssetOrNull<UAnimSequence>(
                FSoftObjectPath(TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Left_Strafe.Left_Strafe"))
            )
        );

        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::WalkBackwards,
            LoadAssetOrNull<UAnimSequence>(
                FSoftObjectPath(TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Walk_Backwards.Walk_Backwards"))
            )
        );

        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::RightStrafe,
            LoadAssetOrNull<UAnimSequence>(
                FSoftObjectPath(TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Right_Strafe.Right_Strafe"))
            )
        );

        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::LeftStrafeRun,
            LoadAssetOrNull<UAnimSequence>(
                FSoftObjectPath(TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Left_Strafe_Run.Left_Strafe_Run"))
            )
        );

        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::RightStrafeRun,
            LoadAssetOrNull<UAnimSequence>(
                FSoftObjectPath(TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Right_Strafe_Run.Right_Strafe_Run"))
            )
        );
    }


    // Networked games should let GameMode/GameFramework own possession.
    AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void APlayableActor::SetupPlayerInputComponent(
    UInputComponent* PlayerInput
)
{
    Super::SetupPlayerInputComponent(PlayerInput);

    if (UPlayerInputComponent* InputComp =
        FindComponentByClass<UPlayerInputComponent>())
    {
        InputComp->BindInput(PlayerInput);
    }
}
