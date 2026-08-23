//
//  ActorRegistry.cpp
//  CaribbeanUntitled (Mac)
//
//  Created by Karl-Johan Bailey on 8/12/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#include "ActorRegistry.h"

#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"


void UActorRegistry::RegisterActor(AActor* Actor)
{
    if (!IsValid(Actor))
    {

        return;
    }

    if (AllActors.Contains(Actor))
    {

        return;
    }

    AllActors.Add(Actor);


    BuildIndexesForActor(Actor);
}


void UActorRegistry::UnregisterActor(AActor* Actor)
{
    if (!Actor)
    {

        return;
    }

    const bool bWasRegistered = AllActors.Contains(Actor);

    AllActors.Remove(Actor);

    RemoveActorFromIndexes(Actor);

    if (bWasRegistered)
    {
    }
    else
    {
    }
}


bool UActorRegistry::IsRegistered(
    const AActor* Actor
) const
{
    return Actor &&
           AllActors.Contains(
               const_cast<AActor*>(Actor)
           );
}


void UActorRegistry::BuildIndexesForActor(
    AActor* Actor
)
{
    if (!IsValid(Actor))
    {
        return;
    }

    TArray<UActorComponent*> Components;

    Actor->GetComponents(Components);


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
    AActor* Actor,
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

    TSet<TWeakObjectPtr<AActor>>& Index =
        ComponentIndexes.FindOrAdd(ComponentClass);

    const bool bWasAlreadyIndexed =
        Index.Contains(Actor);

    Index.Add(Actor);

    if (!bWasAlreadyIndexed)
    {
    }
}


void UActorRegistry::ComponentRemoved(
    AActor* Actor,
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

    TSet<TWeakObjectPtr<AActor>>* Index =
        ComponentIndexes.Find(ComponentClass);

    if (!Index)
    {

        return;
    }

    const int32 PreviousCount = Index->Num();

    Index->Remove(Actor);


    if (Index->Num() == 0)
    {
        ComponentIndexes.Remove(ComponentClass);

    }
}


const TSet<TWeakObjectPtr<AActor>>*
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
    AActor* Actor
)
{
    if (!Actor)
    {
        return;
    }

    int32 RemovedFromIndexes = 0;

    for (auto It = ComponentIndexes.CreateIterator();
         It;
         ++It)
    {
        TSet<TWeakObjectPtr<AActor>>& Index = It.Value();

        if (Index.Remove(Actor) > 0)
        {
            ++RemovedFromIndexes;
        }

        if (Index.Num() == 0)
        {
            It.RemoveCurrent();
        }
    }

}


void UActorRegistry::CleanupInvalidActors()
{
    int32 RemovedActors = 0;
    int32 RemovedIndexedActors = 0;

    for (auto It = AllActors.CreateIterator();
         It;
         ++It)
    {
        if (!It->IsValid())
        {
            It.RemoveCurrent();
            ++RemovedActors;
        }
    }

    for (auto It = ComponentIndexes.CreateIterator();
         It;
         ++It)
    {
        TSet<TWeakObjectPtr<AActor>>& Index = It.Value();

        for (auto ActorIt = Index.CreateIterator();
             ActorIt;
             ++ActorIt)
        {
            if (!ActorIt->IsValid())
            {
                ActorIt.RemoveCurrent();
                ++RemovedIndexedActors;
            }
        }

        if (Index.Num() == 0)
        {
            It.RemoveCurrent();
        }
    }

    if (RemovedActors > 0 ||
        RemovedIndexedActors > 0)
    {
    }
}


void UActorRegistry::Deinitialize()
{

    AllActors.Empty();
    ComponentIndexes.Empty();

    Super::Deinitialize();
}

