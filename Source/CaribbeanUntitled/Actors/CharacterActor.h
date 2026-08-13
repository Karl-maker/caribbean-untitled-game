//
//  CharacterActor.h
//  CaribbeanUntitled (Mac)
//
//  Created by Family on 8/13/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#pragma once

#include "CoreMinimal.h"
#include "K98Actor.h"
#include "CharacterActor.generated.h"

class UControlComponent;
class USkeletalMeshComponent;
class UCapsuleComponent;

UCLASS()
class CARIBBEANUNTITLED_API ACharacterActor : public AK98Actor
{
    GENERATED_BODY()

public:

    ACharacterActor();

protected:

    UPROPERTY(VisibleAnywhere, Category = "Actor")
    TObjectPtr<UControlComponent> ControlComponent;
    
    UPROPERTY(VisibleAnywhere, Category = "Actor")
    TObjectPtr<USkeletalMeshComponent> MeshComponent;
    
    UPROPERTY(VisibleAnywhere, Category = "Actor")
    TObjectPtr<UCapsuleComponent> CollisionComponent;
};
