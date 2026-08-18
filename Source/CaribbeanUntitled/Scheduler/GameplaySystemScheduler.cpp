//
// GameplaySystemScheduler.cpp
// CaribbeanUntitled
//

#include "GameplaySystemScheduler.h"

#include "../Systems/MovementSystem.h"
#include "../Systems/CameraSystem.h"
#include "../Systems/PerceptionSystem.h"
#include "../Systems/BrainSystem.h"
#include "../Systems/ActionSystem.h"
#include "../Systems/StaminaRegenerationSystem.h"

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
    PerceptionSystem = NewObject<UPerceptionSystem>(this);
    BrainSystem = NewObject<UBrainSystem>(this);
    ActionSystem = NewObject<UActionSystem>(this);
    StaminaRegenerationSystem = NewObject<UStaminaRegenerationSystem>(this);

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

    if (MovementSystem)
    {
        MovementSystem->Cleanup();
    }
    
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

    if (PerceptionSystem) PerceptionSystem->Tick(GetWorld(), DeltaTime);
    if (BrainSystem) BrainSystem->Tick(DeltaTime);
    if (ActionSystem) ActionSystem->Tick(DeltaTime);
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
    
    if(StaminaRegenerationSystem) StaminaRegenerationSystem->Tick(DeltaTime);
}


// ================================================================
// MOVEMENT
// ================================================================

void UGameplaySystemScheduler::TickMovement(float DeltaTime)
{
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
