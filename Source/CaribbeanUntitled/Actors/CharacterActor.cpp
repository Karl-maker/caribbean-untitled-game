//
// CharacterActor.cpp
//

#include "CharacterActor.h"

#include "../Components/ControlComponent.h"
#include "../Components/StatsComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


ACharacterActor::ACharacterActor()
{
    PrimaryActorTick.bCanEverTick = false;


    // ============================================================
    // COMPONENTS
    // ============================================================

    ControlComponent =
        CreateDefaultSubobject<UControlComponent>(
            TEXT("Control")
        );

    StatsComponent =
        CreateDefaultSubobject<UStatsComponent>(
            TEXT("Stats")
        );


    // ============================================================
    // COLLISION
    // ============================================================

    GetCapsuleComponent()->InitCapsuleSize(
        42.0f,
        96.0f
    );


    // ============================================================
    // CHARACTER MOVEMENT
    // ============================================================

    UCharacterMovementComponent* Movement =
        GetCharacterMovement();

    if (!Movement)
    {
        return;
    }


    // Walking

    Movement->MaxWalkSpeed =
        MaxWalkSpeed;

    Movement->MaxWalkSpeedCrouched =
        MaxWalkSpeedCrouched;

    Movement->MaxAcceleration =
        MaxAcceleration;

    Movement->BrakingDecelerationWalking =
        BrakingDecelerationWalking;

    Movement->GroundFriction =
        GroundFriction;


    // Air

    Movement->AirControl =
        AirControl;

    Movement->AirControlBoostMultiplier =
        AirControlBoostMultiplier;

    Movement->AirControlBoostVelocityThreshold =
        AirControlBoostVelocityThreshold;


    // Jump / Fall

    Movement->JumpZVelocity =
        JumpZVelocity;

    Movement->GravityScale =
        GravityScale;


    // Crouch

    Movement->NavAgentProps.bCanCrouch =
        true;


    // Rotation

    bUseControllerRotationYaw =
        false;

    Movement->bOrientRotationToMovement =
        bOrientRotationToMovement;

    Movement->RotationRate =
        FRotator(
            0.0f,
            RotationRateYaw,
            0.0f
        );
}
