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
#include "../Systems/CombatDetectionSystem.h"

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
    CombatDetectionSystem = NewObject<UCombatDetectionSystem>(this);
    
    PerceptionSystem = GetWorld() ? GetWorld()->GetSubsystem<UPerceptionSystem>() : nullptr;
    BrainSystem = GetWorld() ? GetWorld()->GetSubsystem<UBrainSystem>() : nullptr;
    ActionSystem = GetWorld() ? GetWorld()->GetSubsystem<UActionSystem>() : nullptr;
    StaminaRegenerationSystem = GetWorld() ? GetWorld()->GetSubsystem<UStaminaRegenerationSystem>() : nullptr;

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

    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    const ENetMode NetMode = World->GetNetMode();

    const bool bIsClient =
        NetMode == NM_Client ||
        NetMode == NM_Standalone;

    const bool bIsServer =
        NetMode == NM_Standalone ||
        NetMode == NM_DedicatedServer ||
        NetMode == NM_ListenServer;

    if (bIsClient) TickInput(DeltaTime);

    if (bIsServer) TickGameplay(DeltaTime);

    if (bIsServer || bIsClient) TickMovement(DeltaTime);
    
    if (bIsClient)
        if (CameraSystem) CameraSystem->Tick(GetWorld(), DeltaTime);
    
    if (bIsServer) TickPhysics(DeltaTime);

    if (bIsServer) TickPostPhysics(DeltaTime);

    if (bIsServer) TickTransform(DeltaTime);
}


// ================================================================
// INPUT
// ================================================================

void UGameplaySystemScheduler::TickInput(float DeltaTime)
{

}


// ================================================================
// GAMEPLAY
// ================================================================

void UGameplaySystemScheduler::TickGameplay(float DeltaTime)
{
    if (PerceptionSystem) PerceptionSystem->Tick(GetWorld(), DeltaTime);
    if (BrainSystem) BrainSystem->Tick(DeltaTime);
    if (ActionSystem) ActionSystem->Tick(DeltaTime);
    if (CombatDetectionSystem) CombatDetectionSystem->Tick(DeltaTime);
}


// ================================================================
// MOVEMENT
// ================================================================

void UGameplaySystemScheduler::TickMovement(float DeltaTime)
{
    if (MovementSystem) MovementSystem->Tick(DeltaTime);
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
