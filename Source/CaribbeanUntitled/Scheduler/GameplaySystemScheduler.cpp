//
// GameplaySystemScheduler.cpp
// CaribbeanUntitled
//

#include "GameplaySystemScheduler.h"
#include "../Systems/PhysicsSystem.h"
#include "../Systems/MovementSystem.h"


void UGameplaySystemScheduler::Initialize(
    FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    MovementSystem = NewObject<UMovementSystem>(this);
    PhysicsSystem = NewObject<UPhysicsSystem>(this);
}


void UGameplaySystemScheduler::Deinitialize()
{
    MovementSystem = nullptr;
    PhysicsSystem = nullptr;
    Super::Deinitialize();
}


void UGameplaySystemScheduler::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    // Explicit execution order.

    TickInput(DeltaTime);

    TickGameplay(DeltaTime);

    TickMovement(DeltaTime);

    TickPhysics(DeltaTime);

    TickPostPhysics(DeltaTime);

    TickTransform(DeltaTime);
}


void UGameplaySystemScheduler::TickInput(float DeltaTime)
{
    // Input systems.
}


void UGameplaySystemScheduler::TickGameplay(float DeltaTime)
{
    // Gameplay systems.
    //
    // Stamina
    // Health
    // Abilities
    // Status effects
}


void UGameplaySystemScheduler::TickMovement(float DeltaTime)
{
    if (!MovementSystem)
    {
        return;
    }

    MovementSystem->Tick(DeltaTime);
}


void UGameplaySystemScheduler::TickPhysics(float DeltaTime)
{
    if (!PhysicsSystem)
    {
        return;
    }

    PhysicsSystem->Tick(DeltaTime);
}


void UGameplaySystemScheduler::TickPostPhysics(float DeltaTime)
{
    // Collision / gameplay reactions.
}


void UGameplaySystemScheduler::TickTransform(float DeltaTime)
{
    // Synchronize MotionComponent -> UE transforms.
}
