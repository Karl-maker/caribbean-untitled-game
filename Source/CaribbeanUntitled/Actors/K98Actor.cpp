//
//  GameActor.cpp
//  CaribbeanUntitled (Mac)
//
//  Created by Karl-Johan Bailey on 8/12/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#include "K98Actor.h"
#include "Engine/World.h"
#include "../Systems/ActorRegistry.h"

AK98Actor::AK98Actor()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    RootComponent = Root;
}

void AK98Actor::NotifyRegistryComponentAdded(UActorComponent* Component)
{
    if (UActorRegistry* Registry = GetWorld()->GetSubsystem<UActorRegistry>())
    {
        Registry->ComponentAdded(this, Component);
    }
}


bool AK98Actor::RemoveComponent(
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

