//
//  ActorRegistry.h
//  CaribbeanUntitled (Mac)
//
//  Created by Karl-Johan Bailey on 8/12/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorRegistry.generated.h"

// Forward declaration - breaks the circular dependency
class AGameActor;

UCLASS()
class CARIBBEANUNTITLED_API UActorRegistry : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    void RegisterActor(AGameActor* Actor);

    void UnregisterActor(AGameActor* Actor);

    bool IsRegistered(const AGameActor* Actor) const;

    /**
     * Returns every registered gameplay Actor.
     */
    const TSet<TWeakObjectPtr<AGameActor>>& GetAllActors() const
    {
        return AllActors;
    }

    /**
     * Returns the internal index for a component class.
     *
     * This is O(1) lookup and O(1) average insertion/removal.
     */
    const TSet<TWeakObjectPtr<AGameActor>>*
    GetComponentIndex(UClass* ComponentClass) const;

    /**
     * Called when a component is added to an Actor.
     */
    void ComponentAdded(
        AGameActor* Actor,
        UActorComponent* Component
    );

    /**
     * Called when a component is removed from an Actor.
     */
    void ComponentRemoved(
        AGameActor* Actor,
        UActorComponent* Component
    );

    template<typename T>
    const TSet<TWeakObjectPtr<AGameActor>>*
    GetActorsWithComponent() const
    {
        static_assert(
            TIsDerivedFrom<T, UActorComponent>::IsDerived,
            "T must derive from UActorComponent."
        );

        return GetComponentIndex(T::StaticClass());
    }

protected:

    virtual void Deinitialize() override;

private:

    /**
     * Master Actor set.
     */
    TSet<TWeakObjectPtr<AGameActor>> AllActors;

    /**
     * Component -> Actor index.
     *
     * Example:
     *
     * UMeshComponent:
     *     ActorA
     *     ActorB
     *     ActorC
     *
     * UPhysicsComponent:
     *     ActorA
     *     ActorC
     */
    TMap<
        TObjectPtr<UClass>,
        TSet<TWeakObjectPtr<AGameActor>>
    > ComponentIndexes;

    void BuildIndexesForActor(AGameActor* Actor);

    void RemoveActorFromIndexes(AGameActor* Actor);

    void CleanupInvalidActors();
};
