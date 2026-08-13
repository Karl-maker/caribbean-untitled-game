//
//  GameActor.h
//  CaribbeanUntitled (Mac)
//
//  Created by Karl-Johan Bailey on 8/12/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "K98Actor.generated.h"

UCLASS()
class CARIBBEANUNTITLED_API AK98Actor : public AActor
{
    GENERATED_BODY()

public:

    AK98Actor();

    /**
     * Dynamically creates a component and attaches it to this Actor.
     */
    template<typename T>
    T* AddComponent();

    /**
     * Gets the first component of the requested type.
     */
    template<typename T>
    T* GetComponent() const;

    /**
     * Gets all components of the requested type.
     */
    template<typename T>
    void GetComponentsOfType(TArray<T*>& OutComponents) const;

    /**
     * Checks whether this Actor contains a component of the requested type.
     */
    template<typename T>
    bool HasComponent() const;

    /**
     * Removes and destroys a runtime component.
     */
    bool RemoveComponent(UActorComponent* Component);

protected:

    /**
     * Every scene-based component can attach to this.
     */
    UPROPERTY(VisibleAnywhere, Category = "Actor")
    TObjectPtr<USceneComponent> Root;

private:
    // Helper to notify registry about component addition
    void NotifyRegistryComponentAdded(UActorComponent* Component);
};


template<typename T>
T* AK98Actor::AddComponent()
{
    static_assert(
        TIsDerivedFrom<T, UActorComponent>::IsDerived,
        "T must derive from UActorComponent."
    );

    T* Component = NewObject<T>(this);

    if (!Component)
    {
        return nullptr;
    }

    AddInstanceComponent(Component);

    Component->RegisterComponent();

    if constexpr (TIsDerivedFrom<T, USceneComponent>::IsDerived)
    {
        if (Root)
        {
            Component->AttachToComponent(
                Root,
                FAttachmentTransformRules::KeepRelativeTransform
            );
        }
    }

    // Use the helper function
    NotifyRegistryComponentAdded(Component);

    return Component;
}


template<typename T>
T* AK98Actor::GetComponent() const
{
    static_assert(
        TIsDerivedFrom<T, UActorComponent>::IsDerived,
        "T must derive from UActorComponent."
    );

    return FindComponentByClass<T>();
}


template<typename T>
void AK98Actor::GetComponentsOfType(TArray<T*>& OutComponents) const
{
    static_assert(
        TIsDerivedFrom<T, UActorComponent>::IsDerived,
        "T must derive from UActorComponent."
    );

    OutComponents.Reset();

    GetComponents<T>(OutComponents);
}


template<typename T>
bool AK98Actor::HasComponent() const
{
    return GetComponent<T>() != nullptr;
}
