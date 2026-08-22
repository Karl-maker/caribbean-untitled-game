#include "CombatInputSystem.h"

#include "ActorRegistry.h"
#include "../Components/ControlComponent.h"
#include "../Components/CombatComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

//=========================================================================
// TICK
//=========================================================================

void UCombatInputSystem::Tick(float DeltaTime)
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

    UE_LOG(
        LogTemp,
        Verbose,
        TEXT("CombatInputSystem: Tick | World=%s"),
        *GetNameSafe(World)
    );

    UActorRegistry* Registry = GetActorRegistry();

    if (!Registry)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatInputSystem: No ActorRegistry"));
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors =
        Registry->GetActorsWithComponent<UControlComponent>();

    if (!Actors)
    {
        UE_LOG(LogTemp, Verbose, TEXT("CombatInputSystem: No actors with ControlComponent"));
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *Actors)
    {
        AActor* Actor = WeakActor.Get();

        if (!IsValid(Actor))
        {
            continue;
        }

        ProcessActor(Actor);
    }
}

//=========================================================================
// STAT ID
//=========================================================================

TStatId UCombatInputSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(
        UCombatInputSystem,
        STATGROUP_Tickables
    );
}

//=========================================================================
// PROCESS ACTOR
//=========================================================================

void UCombatInputSystem::ProcessActor(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }

    UControlComponent* Control =
        Actor->FindComponentByClass<UControlComponent>();

    UCombatComponent* Combat =
        Actor->FindComponentByClass<UCombatComponent>();

    if (!IsValid(Control) || !IsValid(Combat))
    {
        UE_LOG(
            LogTemp,
            Verbose,
            TEXT("CombatInputSystem: Missing Control or Combat on %s"),
            *GetNameSafe(Actor)
        );
        return;
    }

    //=====================================================================
    // LIGHT ATTACK
    //=====================================================================

    const bool bLightAttackPressed =
        Control->WantsToLightAttack();

    bool& bWasLightAttackPressed =
        PreviousLightAttackState.FindOrAdd(Control, false);

    UE_LOG(
        LogTemp,
        Verbose,
        TEXT("CombatInputSystem: %s LightAttack=%d Prev=%d"),
        *GetNameSafe(Actor),
        bLightAttackPressed,
        bWasLightAttackPressed
    );

    if (bLightAttackPressed && !bWasLightAttackPressed)
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("CombatInputSystem: Light attack request detected for %s"),
            *GetNameSafe(Actor)
        );
        Combat->RequestAttack(ECombatAction::LightAttack);
    }

    bWasLightAttackPressed = bLightAttackPressed;

    //=====================================================================
    // HEAVY ATTACK
    //=====================================================================

    const bool bHeavyAttackPressed =
        Control->WantsToHeavyAttack();

    bool& bWasHeavyAttackPressed =
        PreviousHeavyAttackState.FindOrAdd(Control, false);

    UE_LOG(
        LogTemp,
        Verbose,
        TEXT("CombatInputSystem: %s HeavyAttack=%d Prev=%d"),
        *GetNameSafe(Actor),
        bHeavyAttackPressed,
        bWasHeavyAttackPressed
    );

    if (bHeavyAttackPressed && !bWasHeavyAttackPressed)
    {
        Combat->RequestAttack(ECombatAction::HeavyAttack);
    }

    bWasHeavyAttackPressed = bHeavyAttackPressed;
}

//=========================================================================
// GET ACTOR REGISTRY
//=========================================================================

UActorRegistry* UCombatInputSystem::GetActorRegistry() const
{
    UWorld* World = GetWorld();

    return World
        ? World->GetSubsystem<UActorRegistry>()
        : nullptr;
}

//=========================================================================
// CLEANUP
//=========================================================================

void UCombatInputSystem::Cleanup()
{
    PreviousLightAttackState.Empty();
    PreviousHeavyAttackState.Empty();
}
