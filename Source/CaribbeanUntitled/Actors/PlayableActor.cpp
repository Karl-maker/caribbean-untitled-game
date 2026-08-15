//
// PlayableActor.cpp
//

#include "PlayableActor.h"
#include "../Components/PlayerInputComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

APlayableActor::APlayableActor()
{
    PlayerInputComponent =
        CreateDefaultSubobject<UPlayerInputComponent>(
            TEXT("PlayerInputComponent")
        );

    // ============================================================
    // THIRD-PERSON CAMERA
    // ============================================================

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;      // distance behind character
    CameraBoom->bUsePawnControlRotation = true; // boom rotates with the controller (mouse look)
    CameraBoom->bDoCollisionTest = true;        // pulls camera in if something blocks it

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false; // camera itself doesn't rotate independently; the boom does
}

void APlayableActor::SetupPlayerInputComponent(UInputComponent* PlayerInput)
{
    Super::SetupPlayerInputComponent(PlayerInput);

    if (UPlayerInputComponent* InputComp = FindComponentByClass<UPlayerInputComponent>())
    {
        InputComp->BindInput(PlayerInput);
    }
}
