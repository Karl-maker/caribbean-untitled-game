//
// CharacterActor.h
//

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterActor.generated.h"

class UControlComponent;
class UStatsComponent;
class UActorRegistry;

UCLASS(Config=Game)
class CARIBBEANUNTITLED_API ACharacterActor : public ACharacter
{
    GENERATED_BODY()

public:

    ACharacterActor();

protected:
    
    virtual void PostInitializeComponents() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;

    // ============================================================
    // COMPONENTS
    // ============================================================

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor"
    )
    TObjectPtr<UControlComponent> ControlComponent;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor"
    )
    TObjectPtr<UStatsComponent> StatsComponent;


    // ============================================================
    // MOVEMENT
    // ============================================================

    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Walking",
        meta=(ClampMin="0.0")
    )
    float MaxWalkSpeed = 300.0f;


    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Walking",
        meta=(ClampMin="0.0")
    )
    float MaxWalkSpeedCrouched = 150.0f;


    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Walking",
        meta=(ClampMin="0.0")
    )
    float MaxAcceleration = 2048.0f;


    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Walking",
        meta=(ClampMin="0.0")
    )
    float BrakingDecelerationWalking = 2048.0f;


    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Walking",
        meta=(ClampMin="0.0")
    )
    float GroundFriction = 8.0f;


    // ============================================================
    // AIR MOVEMENT
    // ============================================================

    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Air",
        meta=(ClampMin="0.0")
    )
    float AirControl = 0.35f;


    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Air",
        meta=(ClampMin="0.0")
    )
    float AirControlBoostMultiplier = 2.0f;


    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Air",
        meta=(ClampMin="0.0")
    )
    float AirControlBoostVelocityThreshold = 25.0f;


    // ============================================================
    // JUMP / FALL
    // ============================================================

    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Jump",
        meta=(ClampMin="0.0")
    )
    float JumpZVelocity = 500.0f;


    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Jump"
    )
    float GravityScale = 1.0f;


    // ============================================================
    // ROTATION
    // ============================================================

    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Rotation"
    )
    bool bOrientRotationToMovement = true;


    UPROPERTY(
        EditAnywhere,
        Config,
        Category="Movement|Rotation"
    )
    float RotationRateYaw = 720.0f;
};
