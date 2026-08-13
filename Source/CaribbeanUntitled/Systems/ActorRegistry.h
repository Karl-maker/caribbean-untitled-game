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
class AK98Actor;

UCLASS()
class CARIBBEANUNTITLED_API UActorRegistry : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    void RegisterActor(AK98Actor* Actor);

    void UnregisterActor(AK98Actor* Actor);

    bool IsRegistered(const AK98Actor* Actor) const;

    /**
     * Returns every registered gameplay Actor.
     */
    const TSet<TWeakObjectPtr<AK98Actor>>& GetAllActors() const
    {
        return AllActors;
    }

    /**
     * Returns the internal index for a component class.
     *
     * This is O(1) lookup and O(1) average insertion/removal.
     */
    const TSet<TWeakObjectPtr<AK98Actor>>*
    GetComponentIndex(UClass* ComponentClass) const;

    /**
     * Called when a component is added to an Actor.
     */
    void ComponentAdded(
        AK98Actor* Actor,
        UActorComponent* Component
    );

    /**
     * Called when a component is removed from an Actor.
     */
    void ComponentRemoved(
        AK98Actor* Actor,
        UActorComponent* Component
    );

    template<typename T>
    const TSet<TWeakObjectPtr<AK98Actor>>*
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
    TSet<TWeakObjectPtr<AK98Actor>> AllActors;

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
        TSet<TWeakObjectPtr<AK98Actor>>
    > ComponentIndexes;

    void BuildIndexesForActor(AK98Actor* Actor);

    void RemoveActorFromIndexes(AK98Actor* Actor);

    void CleanupInvalidActors();
};
