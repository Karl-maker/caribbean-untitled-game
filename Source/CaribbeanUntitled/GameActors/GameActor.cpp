//
//  GameActor.cpp
//  CaribbeanUntitled (Mac)
//
//  Created by Karl-Johan Bailey on 8/12/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#include "GameActor.h"
#include "../Systems/ActorRegistry.h"

AGameActor::AGameActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    RootComponent = Root;
}

void AGameActor::NotifyRegistryComponentAdded(UActorComponent* Component)
{
    if (UActorRegistry* Registry = GetWorld()->GetSubsystem<UActorRegistry>())
    {
        Registry->ComponentAdded(this, Component);
    }
}


bool AGameActor::RemoveComponent(
    UActorComponent* Component
)
{
    if (!Component ||
        Component->GetOwner() != this)
    {
        return false;
    }

    if (UActorRegistry* Registry =
        GetWorld()->GetSubsystem<UActorRegistry>())
    {
        Registry->ComponentRemoved(
            this,
            Component
        );
    }

    Component->DestroyComponent();

    return true;
}

