#pragma once

#include "CoreMinimal.h"
#include "K98Actor.h"
#include "PlayerActor.generated.h"

class UControlComponent;
// class UCameraComponent;
// etc.

UCLASS()
class CARIBBEANUNTITLED_API APlayerActor : public AK98Actor
{
    GENERATED_BODY()

public:

    APlayerActor();

protected:
//
    UPROPERTY(VisibleAnywhere, Category = "Actor")
    TObjectPtr<UControlComponent> ControlComponent;
    
//     Add the rest of your standard player components here.
//    
//     UPROPERTY(VisibleAnywhere, Category = "Actor")
//     TObjectPtr<UMovementComponent> MovementComponent;
//    
//     UPROPERTY(VisibleAnywhere, Category = "Actor")
//     TObjectPtr<UMeshComponent> MeshComponent;
};
