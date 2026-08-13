//
//  ActorRegistry.cpp
//  CaribbeanUntitled (Mac)
//
//  Created by Karl-Johan Bailey on 8/12/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#include "ActorRegistry.h"
#include "../GameActors/GameActor.h"

void UActorRegistry::RegisterActor(AGameActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }

    // Already registered.
    if (AllActors.Contains(Actor))
    {
        return;
    }

    AllActors.Add(Actor);

    BuildIndexesForActor(Actor);
}


void UActorRegistry::UnregisterActor(AGameActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    AllActors.Remove(Actor);

    RemoveActorFromIndexes(Actor);
}


bool UActorRegistry::IsRegistered(
    const AGameActor* Actor
) const
{
    return Actor &&
           AllActors.Contains(
               const_cast<AGameActor*>(Actor)
           );
}


void UActorRegistry::BuildIndexesForActor(
    AGameActor* Actor
)
{
    if (!IsValid(Actor))
    {
        return;
    }

    TArray<UActorComponent*> Components;

    Actor->GetComponents(Components);  // <-- This needs the full AGameActor definition

    for (UActorComponent* Component : Components)
    {
        if (!IsValid(Component))
        {
            continue;
        }

        ComponentAdded(
            Actor,
            Component
        );
    }
}


void UActorRegistry::ComponentAdded(
    AGameActor* Actor,
    UActorComponent* Component
)
{
    if (!IsValid(Actor) ||
        !IsValid(Component))
    {
        return;
    }

    UClass* ComponentClass = Component->GetClass();

    if (!ComponentClass)
    {
        return;
    }

    TSet<TWeakObjectPtr<AGameActor>>& Index =
        ComponentIndexes.FindOrAdd(ComponentClass);

    Index.Add(Actor);
}


void UActorRegistry::ComponentRemoved(
    AGameActor* Actor,
    UActorComponent* Component
)
{
    if (!Actor || !Component)
    {
        return;
    }

    UClass* ComponentClass = Component->GetClass();

    if (!ComponentClass)
    {
        return;
    }

    TSet<TWeakObjectPtr<AGameActor>>* Index =
        ComponentIndexes.Find(ComponentClass);

    if (!Index)
    {
        return;
    }

    Index->Remove(Actor);

    // Don't keep empty indexes around.
    if (Index->Num() == 0)
    {
        ComponentIndexes.Remove(ComponentClass);
    }
}


const TSet<TWeakObjectPtr<AGameActor>>*
UActorRegistry::GetComponentIndex(
    UClass* ComponentClass
) const
{
    if (!ComponentClass)
    {
        return nullptr;
    }

    return ComponentIndexes.Find(ComponentClass);
}


void UActorRegistry::RemoveActorFromIndexes(
    AGameActor* Actor
)
{
    if (!Actor)
    {
        return;
    }

    for (auto It = ComponentIndexes.CreateIterator();
         It;
         ++It)
    {
        TSet<TWeakObjectPtr<AGameActor>>& Index = It.Value();

        Index.Remove(Actor);

        if (Index.Num() == 0)
        {
            It.RemoveCurrent();
        }
    }
}


void UActorRegistry::CleanupInvalidActors()
{
    for (auto It = AllActors.CreateIterator();
         It;
         ++It)
    {
        if (!It->IsValid())
        {
            It.RemoveCurrent();
        }
    }

    for (auto It = ComponentIndexes.CreateIterator();
         It;
         ++It)
    {
        TSet<TWeakObjectPtr<AGameActor>>& Index = It.Value();

        for (auto ActorIt = Index.CreateIterator();
             ActorIt;
             ++ActorIt)
        {
            if (!ActorIt->IsValid())
            {
                ActorIt.RemoveCurrent();
            }
        }

        if (Index.Num() == 0)
        {
            It.RemoveCurrent();
        }
    }
}


void UActorRegistry::Deinitialize()
{
    AllActors.Empty();
    ComponentIndexes.Empty();

    Super::Deinitialize();
}
