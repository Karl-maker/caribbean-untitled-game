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
#include "../Systems/AnimationSystem.h"
#include "../Systems/StaminaRegenerationSystem.h"
#include "../Systems/StaminaSystem.h"
#include "../Systems/CombatDetectionSystem.h"
#include "../Systems/CombatInputSystem.h"

#include "Engine/World.h"


void UGameplaySystemScheduler::Initialize(
    FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(
        LogTemp,
        Error,
        TEXT(
            "[GameplaySystemScheduler]: INITIALIZE | World=%s | Type=%d"
        ),
        *GetNameSafe(GetWorld()),
        GetWorld()
            ? static_cast<int32>(GetWorld()->WorldType)
            : -1
    );

    MovementSystem = NewObject<UMovementSystem>(this);
    StaminaSystem = NewObject<UStaminaSystem>(this);
    CameraSystem = NewObject<UCameraSystem>(this);
    PerceptionSystem = NewObject<UPerceptionSystem>(this);
    BrainSystem = NewObject<UBrainSystem>(this);
    ActionSystem = NewObject<UActionSystem>(this);
    AnimationSystem = NewObject<UAnimationSystem>(this);
    StaminaRegenerationSystem = NewObject<UStaminaRegenerationSystem>(this);
    CombatDetectionSystem = NewObject<UCombatDetectionSystem>(this);
    CombatInputSystem = NewObject<UCombatInputSystem>(this);
}


void UGameplaySystemScheduler::Deinitialize()
{
    UE_LOG(
        LogTemp,
        Error,
        TEXT(
            "[GameplaySystemScheduler]: DEINITIALIZE | World=%s"
        ),
        *GetNameSafe(GetWorld())
    );

    if (MovementSystem)
    {
        MovementSystem->Cleanup();
    }
    MovementSystem = nullptr;

    if (StaminaSystem)
    {
        StaminaSystem->Cleanup();
    }
    StaminaSystem = nullptr;

    if (PerceptionSystem)
    {
        PerceptionSystem->Cleanup();
    }
    PerceptionSystem = nullptr;

    if (BrainSystem)
    {
        BrainSystem->Cleanup();
    }
    BrainSystem = nullptr;

    if (ActionSystem)
    {
        ActionSystem->Cleanup();
    }
    ActionSystem = nullptr;

    AnimationSystem = nullptr;

    if (StaminaRegenerationSystem)
    {
        StaminaRegenerationSystem->Cleanup();
    }
    StaminaRegenerationSystem = nullptr;
    
    if (CombatInputSystem)
    {
        CombatInputSystem->Cleanup();
    }
    CombatInputSystem = nullptr;
    
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

    UE_LOG(
        LogTemp,
        Verbose,
        TEXT("[GameplaySystemScheduler]: Tick | World=%s | NetMode=%d | Client=%d | Server=%d"),
        *GetNameSafe(World),
        static_cast<int32>(NetMode),
        bIsClient,
        bIsServer
    );

    if (bIsClient) TickInput(World, DeltaTime);

    if (bIsServer) TickGameplay(World, DeltaTime);

    if (bIsServer || bIsClient) TickMovement(World, DeltaTime);
    
    if (bIsClient)
        if (CameraSystem) CameraSystem->Tick(World, DeltaTime);
    
    if (bIsServer) TickPhysics(World, DeltaTime);

    if (bIsServer) TickPostPhysics(World, DeltaTime);

    if (bIsServer || bIsClient) TickTransform(World, DeltaTime);
}


// ================================================================
// INPUT
// ================================================================

void UGameplaySystemScheduler::TickInput(UWorld* World, float DeltaTime)
{
    (void)World;
    (void)DeltaTime;
}


// ================================================================
// GAMEPLAY
// ================================================================

void UGameplaySystemScheduler::TickGameplay(UWorld* World, float DeltaTime)
{
    if (CombatInputSystem) CombatInputSystem->Tick(World, DeltaTime);
    if (PerceptionSystem) PerceptionSystem->Tick(World, DeltaTime);
    if (BrainSystem) BrainSystem->Tick(World, DeltaTime);
    if (ActionSystem) ActionSystem->Tick(World, DeltaTime);
    if (CombatDetectionSystem) CombatDetectionSystem->Tick(World, DeltaTime);
}


// ================================================================
// MOVEMENT
// ================================================================

void UGameplaySystemScheduler::TickMovement(UWorld* World, float DeltaTime)
{
    if (StaminaSystem) StaminaSystem->Tick(World, DeltaTime);
    if (StaminaRegenerationSystem) StaminaRegenerationSystem->Tick(World, DeltaTime);
    if (MovementSystem) MovementSystem->Tick(World, DeltaTime);
}


// ================================================================
// PHYSICS
// ================================================================

void UGameplaySystemScheduler::TickPhysics(UWorld* World, float DeltaTime)
{
    (void)World;
    (void)DeltaTime;
}


// ================================================================
// POST PHYSICS
// ================================================================

void UGameplaySystemScheduler::TickPostPhysics(UWorld* World, float DeltaTime)
{
    (void)World;
    (void)DeltaTime;

    // Collision / gameplay reactions.
}


// ================================================================
// TRANSFORM
// ================================================================

void UGameplaySystemScheduler::TickTransform(UWorld* World, float DeltaTime)
{
    (void)World;
    (void)DeltaTime;
    // Synchronize MotionComponent -> UE transforms.
}
