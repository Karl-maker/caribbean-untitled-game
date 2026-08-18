//
// PlayableActor.cpp
//

#include "PlayableActor.h"

#include "../Components/AnimationRegistryComponent.h"
#include "../Components/PlayerInputComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    template<typename TObjectType>
    TObjectType* LoadAssetOrNull(const TCHAR* AssetPath)
    {
        ConstructorHelpers::FObjectFinder<TObjectType> AssetFinder(AssetPath);
        return AssetFinder.Succeeded() ? AssetFinder.Object : nullptr;
    }

    void ConfigureAnimationDefinition(
        UAnimationRegistryComponent* Registry,
        EAnimationKey Key,
        UAnimSequence* Animation,
        EAnimationBlendMode BlendMode,
        UCurveFloat* BlendCurve = nullptr
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
            Definition.BlendMode = BlendMode;
            Definition.BlendCurve = BlendCurve;
            return;
        }
    }
}

APlayableActor::APlayableActor()
{
    
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

    if (USkeletalMeshComponent* Mesh = GetMesh())
    {
        if (USkeletalMesh* SkeletalMesh = LoadAssetOrNull<USkeletalMesh>(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/X_Bot.X_Bot")
        ))
        {
            Mesh->SetSkeletalMesh(SkeletalMesh);
        }

        static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBlueprintClass(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/X_Bot_Skeleton_AnimBlueprint")
        );

        if (AnimBlueprintClass.Succeeded())
        {
            Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
            Mesh->SetAnimInstanceClass(AnimBlueprintClass.Class);
        }
    }

    if (PlayerInputComponent)
    {
        PlayerInputComponent->DefaultMappingContext =
            LoadAssetOrNull<UInputMappingContext>(
                TEXT("/Game/Input/IMC_Player.IMC_Player")
            );

        PlayerInputComponent->MoveForwardAction =
            LoadAssetOrNull<UInputAction>(
                TEXT("/Game/Input/IA_MoveForward.IA_MoveForward")
            );

        PlayerInputComponent->MoveBackwardAction =
            LoadAssetOrNull<UInputAction>(
                TEXT("/Game/Input/IA_MoveBackward.IA_MoveBackward")
            );

        PlayerInputComponent->MoveLeftAction =
            LoadAssetOrNull<UInputAction>(
                TEXT("/Game/Input/IA_MoveLeft.IA_MoveLeft")
            );

        PlayerInputComponent->MoveRightAction =
            LoadAssetOrNull<UInputAction>(
                TEXT("/Game/Input/IA_MoveRight.IA_MoveRight")
            );

        PlayerInputComponent->LookAction =
            LoadAssetOrNull<UInputAction>(
                TEXT("/Game/Input/IA_Look.IA_Look")
            );

        PlayerInputComponent->JumpAction =
            LoadAssetOrNull<UInputAction>(
                TEXT("/Game/Input/IA_Jump.IA_Jump")
            );

        PlayerInputComponent->SprintAction =
            LoadAssetOrNull<UInputAction>(
                TEXT("/Game/Input/IA_Sprint.IA_Sprint")
            );

        PlayerInputComponent->CrouchAction =
            LoadAssetOrNull<UInputAction>(
                TEXT("/Game/Input/IA_Crouch.IA_Crouch")
            );
    }

    if (AnimationRegistryComponent)
    {
        UAnimSequence* IdleAnimation = LoadAssetOrNull<UAnimSequence>(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Standing_Idle.Standing_Idle")
        );
        UAnimSequence* WalkAnimation = LoadAssetOrNull<UAnimSequence>(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Walking.Walking")
        );
        UAnimSequence* RunAnimation = LoadAssetOrNull<UAnimSequence>(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Medium_Run.Medium_Run")
        );
        UAnimSequence* JumpAnimation = LoadAssetOrNull<UAnimSequence>(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Jumping_Up.Jumping_Up")
        );
        UAnimSequence* FallAnimation = LoadAssetOrNull<UAnimSequence>(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Falling_Idle.Falling_Idle")
        );
        UAnimSequence* LandAnimation = LoadAssetOrNull<UAnimSequence>(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Falling_To_Landing.Falling_To_Landing")
        );
        UAnimSequence* HitAnimation = LoadAssetOrNull<UAnimSequence>(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Reaction_Body_Left.Reaction_Body_Left")
        );
        UAnimSequence* LightPunchAnimation = LoadAssetOrNull<UAnimSequence>(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Light_Left_Punch.Light_Left_Punch")
        );

        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Idle,
            IdleAnimation,
            EAnimationBlendMode::FullBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Walk,
            WalkAnimation,
            EAnimationBlendMode::FullBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Run,
            RunAnimation,
            EAnimationBlendMode::FullBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Sprint,
            RunAnimation,
            EAnimationBlendMode::FullBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Jump,
            JumpAnimation,
            EAnimationBlendMode::FullBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Fall,
            FallAnimation,
            EAnimationBlendMode::FullBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Land,
            LandAnimation,
            EAnimationBlendMode::FullBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Crouch,
            IdleAnimation,
            EAnimationBlendMode::FullBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::LightRightPunch,
            LightPunchAnimation,
            EAnimationBlendMode::UpperBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Hit,
            HitAnimation,
            EAnimationBlendMode::UpperBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Death,
            HitAnimation,
            EAnimationBlendMode::FullBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Interact,
            LightPunchAnimation,
            EAnimationBlendMode::UpperBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Mount,
            JumpAnimation,
            EAnimationBlendMode::FullBody
        );
        ConfigureAnimationDefinition(
            AnimationRegistryComponent,
            EAnimationKey::Dismount,
            LandAnimation,
            EAnimationBlendMode::FullBody
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
