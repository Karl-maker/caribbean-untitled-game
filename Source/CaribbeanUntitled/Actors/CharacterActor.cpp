//
//  CharacterActor.cpp
//  CaribbeanUntitled (Mac)
//
//  Created by Family on 8/13/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#include "CharacterActor.h"

#include "../Components/ControlComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

ACharacterActor::ACharacterActor()
{
    ControlComponent =
        CreateDefaultSubobject<UControlComponent>(
            TEXT("Control")
        );
    
    MeshComponent =
        CreateDefaultSubobject<USkeletalMeshComponent>(
            TEXT("Mesh")
        );
    
    CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(
        TEXT("Collision")
    );


}
