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
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("RegisterActor: Attempted to register an invalid Actor.")
        );

        return;
    }

    if (AllActors.Contains(Actor))
    {
        UE_LOG(
            LogTemp,
            Verbose,
            TEXT("RegisterActor: Actor '%s' is already registered."),
            *GetNameSafe(Actor)
        );

        return;
    }

    AllActors.Add(Actor);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("Actor Registered: '%s' | Class: '%s' | Total Actors: %d"),
        *GetNameSafe(Actor),
        *GetNameSafe(Actor->GetClass()),
        AllActors.Num()
    );

    BuildIndexesForActor(Actor);
}


void UActorRegistry::UnregisterActor(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("UnregisterActor: Attempted to unregister a null Actor.")
        );

        return;
    }

    const bool bWasRegistered = AllActors.Contains(Actor);

    AllActors.Remove(Actor);

    RemoveActorFromIndexes(Actor);

    if (bWasRegistered)
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("Actor Unregistered: '%s' | Remaining Actors: %d"),
            *GetNameSafe(Actor),
            AllActors.Num()
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Verbose,
            TEXT("UnregisterActor: Actor '%s' was not registered."),
            *GetNameSafe(Actor)
        );
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

    UE_LOG(
        LogTemp,
        Verbose,
        TEXT("Building component indexes for Actor '%s' | Components: %d"),
        *GetNameSafe(Actor),
        Components.Num()
    );

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
        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "Component Added: Actor='%s' | "
                "Component='%s' | Type='%s' | Actors In Index: %d"
            ),
            *GetNameSafe(Actor),
            *GetNameSafe(Component),
            *GetNameSafe(ComponentClass),
            Index.Num()
        );
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
        UE_LOG(
            LogTemp,
            Verbose,
            TEXT(
                "Component Removed: "
                "No index exists for Component Type='%s'."
            ),
            *GetNameSafe(ComponentClass)
        );

        return;
    }

    const int32 PreviousCount = Index->Num();

    Index->Remove(Actor);

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "Component Removed: Actor='%s' | "
            "Component='%s' | Type='%s' | Index Count: %d -> %d"
        ),
        *GetNameSafe(Actor),
        *GetNameSafe(Component),
        *GetNameSafe(ComponentClass),
        PreviousCount,
        Index->Num()
    );

    if (Index->Num() == 0)
    {
        ComponentIndexes.Remove(ComponentClass);

        UE_LOG(
            LogTemp,
            Verbose,
            TEXT(
                "Component Index Removed: "
                "Type='%s' had no remaining Actors."
            ),
            *GetNameSafe(ComponentClass)
        );
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

    UE_LOG(
        LogTemp,
        Verbose,
        TEXT(
            "Removed Actor '%s' from %d component indexes."
        ),
        *GetNameSafe(Actor),
        RemovedFromIndexes
    );
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
        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "Registry Cleanup: Removed %d invalid Actors "
                "and %d invalid indexed Actors."
            ),
            RemovedActors,
            RemovedIndexedActors
        );
    }
}


void UActorRegistry::Deinitialize()
{
    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "Actor Registry Deinitializing | "
            "Actors: %d | Component Indexes: %d"
        ),
        AllActors.Num(),
        ComponentIndexes.Num()
    );

    AllActors.Empty();
    ComponentIndexes.Empty();

    Super::Deinitialize();
}

