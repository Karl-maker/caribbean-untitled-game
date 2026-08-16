//
// PlayableActor.cpp
//

#include "PlayableActor.h"

#include "../Components/PlayerInputComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

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
    CameraBoom->bUsePawnControlRotation = true;

    // Camera moves closer when blocked by geometry.
    CameraBoom->bDoCollisionTest = true;


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


    // ============================================================
    // AUTOMATIC PLAYER 0 POSSESSION
    // ============================================================
    //
    // When this actor is placed in the level, Unreal will
    // automatically possess it with Player 0.
    //

    AutoPossessPlayer = EAutoReceiveInput::Player0;
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
