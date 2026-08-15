//
// PlayableActor.h
//

#pragma once

#include "CoreMinimal.h"
#include "CharacterActor.h"
#include "PlayableActor.generated.h"

class UPlayerInputComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS(Config=Game)
class CARIBBEANUNTITLED_API APlayableActor : public ACharacterActor
{
    GENERATED_BODY()

public:

    APlayableActor();

protected:

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor|Components"
    )
    TObjectPtr<UPlayerInputComponent> PlayerInputComponent;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor|Camera"
    )
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor|Camera"
    )
    TObjectPtr<UCameraComponent> FollowCamera;

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInput) override;
};
