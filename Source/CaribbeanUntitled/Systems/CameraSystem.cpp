#include "CameraSystem.h"

#include "../Components/StatsComponent.h"
#include "../Components/ControlComponent.h"
#include "ActorRegistry.h"

#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"


//=============================================================================
// CONSTRUCTOR
//=============================================================================

UCameraSystem::UCameraSystem()
{
}


//=============================================================================
// TICK
//=============================================================================

void UCameraSystem::Tick(
    UWorld* World,
    float DeltaTime
)
{
    if (!World || DeltaTime <= 0.0f)
    {
        return;
    }

    TickCounter++;

    if (TickCounter < UpdateInterval)
    {
        return;
    }

    TickCounter = 0;

    const float EffectiveDeltaTime =
        DeltaTime * static_cast<float>(UpdateInterval);

    UActorRegistry* Registry =
        World->GetSubsystem<UActorRegistry>();

    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors =
        Registry->GetActorsWithComponent<USpringArmComponent>();

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

        ProcessActor(
            Actor,
            EffectiveDeltaTime
        );
    }

    CleanupInvalidActors();
}


//=============================================================================
// UPDATE INTERVAL
//=============================================================================

void UCameraSystem::SetUpdateInterval(int32 InTicks)
{
    UpdateInterval =
        FMath::Max(1, InTicks);
}


//=============================================================================
// PROCESS ACTOR
//=============================================================================

void UCameraSystem::ProcessActor(
    AActor* Actor,
    float DeltaTime
)
{
    if (!IsValid(Actor))
    {
        return;
    }

    USpringArmComponent* CameraBoom =
        Actor->FindComponentByClass<USpringArmComponent>();

    if (!CameraBoom)
    {
        return;
    }

    UStatsComponent* Stats =
        Actor->FindComponentByClass<UStatsComponent>();

    UControlComponent* Control =
        Actor->FindComponentByClass<UControlComponent>();

    if (!Stats || !Control)
    {
        return;
    }

    FCameraState& State =
        CameraStates.FindOrAdd(Actor);


    // ============================================================
    // DETERMINE CAMERA STATE
    // ============================================================

    const bool bRunning =
        IsRunning(Actor);

    const bool bAction =
        IsActionState(Actor);


    // ============================================================
    // SELECT TARGET
    // ============================================================

    if (bRunning)
    {
        State.TargetOffset =
            RunningOffset;

        State.TargetArmLength =
            RunningArmLength;
    }
    else if (bAction)
    {
        State.TargetOffset =
            ActionOffset;

        State.TargetArmLength =
            ActionArmLength;
    }
    else
    {
        State.TargetOffset =
            NormalOffset;

        State.TargetArmLength =
            NormalArmLength;
    }


    // ============================================================
    // INITIALIZE STATE
    // ============================================================

    if (State.CurrentArmLength <= 0.0f)
    {
        State.CurrentArmLength =
            CameraBoom->TargetArmLength;
    }

    if (State.CurrentOffset.IsNearlyZero())
    {
        State.CurrentOffset =
            CameraBoom->GetRelativeLocation();
    }


    // ============================================================
    // POSITION DAMPING
    // ============================================================

    State.CurrentOffset =
        FMath::VInterpTo(
            State.CurrentOffset,
            State.TargetOffset,
            DeltaTime,
            PositionDamping
        );


    // ============================================================
    // ARM LENGTH DAMPING
    // ============================================================

    State.CurrentArmLength =
        FMath::FInterpTo(
            State.CurrentArmLength,
            State.TargetArmLength,
            DeltaTime,
            ArmLengthDamping
        );


    // ============================================================
    // APPLY
    // ============================================================

    CameraBoom->SetRelativeLocation(
        State.CurrentOffset
    );

    CameraBoom->TargetArmLength =
        State.CurrentArmLength;
}


//=============================================================================
// RUNNING
//=============================================================================

bool UCameraSystem::IsRunning(
    AActor* Actor
) const
{
    if (!IsValid(Actor))
    {
        return false;
    }

    UStatsComponent* Stats =
        Actor->FindComponentByClass<UStatsComponent>();

    UControlComponent* Control =
        Actor->FindComponentByClass<UControlComponent>();

    if (!Stats || !Control)
    {
        return false;
    }


    // ============================================================
    // MOVEMENT
    // ============================================================

    const bool bMoving =
        !Control->MoveIntent.IsNearlyZero()
        ||
        !Control->WorldMoveDirection.IsNearlyZero();


    // ============================================================
    // SPRINT INTENT
    // ============================================================

    const bool bWantsSprint =
        Control->SprintIntent > 0.0f;


    // ============================================================
    // SPEED VALIDATION
    // ============================================================

    const bool bSprintIsFaster =
        Stats->GetSprintSpeed() >
        Stats->GetWalkSpeed();


    return
        bMoving &&
        bWantsSprint &&
        bSprintIsFaster;
}


//=============================================================================
// ACTION STATE
//=============================================================================

bool UCameraSystem::IsActionState(
    AActor* Actor
) const
{
    if (!IsValid(Actor))
    {
        return false;
    }

    UControlComponent* Control =
        Actor->FindComponentByClass<UControlComponent>();

    if (!Control)
    {
        return false;
    }


    // ============================================================
    // ACTION INTENTS
    // ============================================================

    return
        Control->GetJumpIntent() > 0.0f
        ||
        Control->GetDashIntent() > 0.0f
        ||
        Control->GetAimIntent() > 0.0f
        ||
        Control->GetFireIntent() > 0.0f
        ||
        Control->GetReloadIntent() > 0.0f;
}


//=============================================================================
// CLEANUP
//=============================================================================

void UCameraSystem::CleanupInvalidActors()
{
    for (auto It = CameraStates.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid())
        {
            It.RemoveCurrent();
        }
    }
}
