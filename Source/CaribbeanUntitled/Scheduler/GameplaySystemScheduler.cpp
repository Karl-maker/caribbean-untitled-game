//
// GameplaySystemScheduler.cpp
// CaribbeanUntitled
//

#include "GameplaySystemScheduler.h"

#include "../Systems/MovementSystem.h"
#include "../Systems/CameraSystem.h"

#include "Engine/World.h"


void UGameplaySystemScheduler::Initialize(
    FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(
        LogTemp,
        Error,
        TEXT(
            "[Scheduler] INITIALIZE | World=%s | Type=%d"
        ),
        *GetNameSafe(GetWorld()),
        GetWorld()
            ? static_cast<int32>(GetWorld()->WorldType)
            : -1
    );

    MovementSystem = NewObject<UMovementSystem>(this);
    CameraSystem = NewObject<UCameraSystem>(this);

}


void UGameplaySystemScheduler::Deinitialize()
{
    UE_LOG(
        LogTemp,
        Error,
        TEXT(
            "[Scheduler] DEINITIALIZE | World=%s"
        ),
        *GetNameSafe(GetWorld())
    );

    MovementSystem = nullptr;

    Super::Deinitialize();
}


void UGameplaySystemScheduler::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }


    // ============================================================
    // EXPLICIT EXECUTION ORDER
    // ============================================================

    TickInput(DeltaTime);

    TickGameplay(DeltaTime);

    TickMovement(DeltaTime);

    TickPhysics(DeltaTime);

    TickPostPhysics(DeltaTime);

    TickTransform(DeltaTime);
}


// ================================================================
// INPUT
// ================================================================

void UGameplaySystemScheduler::TickInput(float DeltaTime)
{


    // Input systems.
}


// ================================================================
// GAMEPLAY
// ================================================================

void UGameplaySystemScheduler::TickGameplay(float DeltaTime)
{

    // Gameplay systems.
    //
    // Stamina
    // Health
    // Abilities
    // Status effects
}


// ================================================================
// MOVEMENT
// ================================================================

void UGameplaySystemScheduler::TickMovement(float DeltaTime)
{

    // ------------------------------------------------------------
    // CONTROL TEST SYSTEM
    // ------------------------------------------------------------

    if (MovementSystem) MovementSystem->Tick(DeltaTime);
    if (CameraSystem) CameraSystem->Tick(GetWorld(), DeltaTime);
}


// ================================================================
// PHYSICS
// ================================================================

void UGameplaySystemScheduler::TickPhysics(float DeltaTime)
{

}


// ================================================================
// POST PHYSICS
// ================================================================

void UGameplaySystemScheduler::TickPostPhysics(float DeltaTime)
{


    // Collision / gameplay reactions.
}


// ================================================================
// TRANSFORM
// ================================================================

void UGameplaySystemScheduler::TickTransform(float DeltaTime)
{


    // Synchronize MotionComponent -> UE transforms.
}
