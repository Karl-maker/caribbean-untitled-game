#include "CombatInputSystem.h"

#include "ActorRegistry.h"
#include "../Components/ControlComponent.h"
#include "../Components/CombatComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

//=========================================================================
// TICK
//=========================================================================

void UCombatInputSystem::Tick(UWorld* World, float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    if (!World)
    {
        return;
    }

    UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();

    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors =
        Registry->GetActorsWithComponent<UControlComponent>();

    if (!Actors)
    {
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
        return;
    }

    //=====================================================================
    // LIGHT ATTACK
    //=====================================================================

    const bool bLightAttackPressed =
        Control->WantsToLightAttack();

    bool& bWasLightAttackPressed =
        PreviousLightAttackState.FindOrAdd(Control, false);


    if (bLightAttackPressed && !bWasLightAttackPressed)
    {
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


    if (bHeavyAttackPressed && !bWasHeavyAttackPressed)
    {
        Combat->RequestAttack(ECombatAction::HeavyAttack);
    }

    bWasHeavyAttackPressed = bHeavyAttackPressed;
}

//=========================================================================
// GET ACTOR REGISTRY
//=========================================================================


//=========================================================================
// CLEANUP
//=========================================================================

void UCombatInputSystem::Cleanup()
{
    PreviousLightAttackState.Empty();
    PreviousHeavyAttackState.Empty();
}
