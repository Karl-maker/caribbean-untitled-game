//
//  ActorRegistry.cpp
//  CaribbeanUntitled (Mac)
//
//  Created by Karl-Johan Bailey on 8/12/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#include "ActorRegistry.h"
#include "../Actors/K98Actor.h"

void UActorRegistry::RegisterActor(AK98Actor* Actor)
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


void UActorRegistry::UnregisterActor(AK98Actor* Actor)
{
    if (!Actor)
    {
        return;
    }

    AllActors.Remove(Actor);

    RemoveActorFromIndexes(Actor);
}


bool UActorRegistry::IsRegistered(
    const AK98Actor* Actor
) const
{
    return Actor &&
           AllActors.Contains(
               const_cast<AK98Actor*>(Actor)
           );
}


void UActorRegistry::BuildIndexesForActor(
     AK98Actor* Actor
)
{
    if (!IsValid(Actor))
    {
        return;
    }

    TArray<UActorComponent*> Components;

    Actor->GetComponents(Components);  // <-- This needs the full AK98Actor definition

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
    AK98Actor* Actor,
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

    TSet<TWeakObjectPtr<AK98Actor>>& Index =
        ComponentIndexes.FindOrAdd(ComponentClass);

    Index.Add(Actor);
}


void UActorRegistry::ComponentRemoved(
    AK98Actor* Actor,
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

    TSet<TWeakObjectPtr<AK98Actor>>* Index =
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


const TSet<TWeakObjectPtr<AK98Actor>>*
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
   AK98Actor* Actor
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
        TSet<TWeakObjectPtr<AK98Actor>>& Index = It.Value();

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
        TSet<TWeakObjectPtr<AK98Actor>>& Index = It.Value();

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
