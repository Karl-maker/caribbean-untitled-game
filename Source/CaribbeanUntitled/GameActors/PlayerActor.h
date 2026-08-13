#pragma once

#include "CoreMinimal.h"
#include "GameActor.h"
#include "PlayerActor.generated.h"

class UControlComponent;
// class UCameraComponent;
// etc.

UCLASS()
class CARIBBEANUNTITLED_API APlayerActor : public AGameActor
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
