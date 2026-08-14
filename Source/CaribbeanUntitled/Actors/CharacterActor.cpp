//
//  CharacterActor.cpp
//  CaribbeanUntitled (Mac)
//
//  Created by Family on 8/13/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#include "CharacterActor.h"

#include "../Components/ControlComponent.h"
#include "../Components/StatsComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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

    
    StatsComponent = CreateDefaultSubobject<UStatsComponent>(
        TEXT("Stats")
    );

    CharacterMovementComponent = CreateDefaultSubobject<UCharacterMovementComponent>(
        TEXT("Movement")
    );
}
