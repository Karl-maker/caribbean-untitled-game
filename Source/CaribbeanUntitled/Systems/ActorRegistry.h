#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorRegistry.generated.h"

class AActor;
class UActorComponent;

UCLASS()
class CARIBBEANUNTITLED_API UActorRegistry : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    void RegisterActor(AActor* Actor);
    void UnregisterActor(AActor* Actor);

    bool IsRegistered(const AActor* Actor) const;

    const TSet<TWeakObjectPtr<AActor>>& GetAllActors() const
    {
        return AllActors;
    }

    const TSet<TWeakObjectPtr<AActor>>*
    GetComponentIndex(UClass* ComponentClass) const;

    void ComponentAdded(
        AActor* Actor,
        UActorComponent* Component
    );

    void ComponentRemoved(
        AActor* Actor,
        UActorComponent* Component
    );

    template<typename T>
    const TSet<TWeakObjectPtr<AActor>>*
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

    TSet<TWeakObjectPtr<AActor>> AllActors;

    TMap<
        TObjectPtr<UClass>,
        TSet<TWeakObjectPtr<AActor>>
    > ComponentIndexes;

    void BuildIndexesForActor(AActor* Actor);

    void RemoveActorFromIndexes(AActor* Actor);

    void CleanupInvalidActors();
};
