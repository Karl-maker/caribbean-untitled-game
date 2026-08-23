//
//  StaminaSystem.h
//  CaribbeanUntitled (Mac)
//
//  Created by Family on 8/20/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StaminaSystem.generated.h"

class AActor;
class ACharacter;
class UCharacterMovementComponent;
class UControlComponent;
class UStatsComponent;

UCLASS()
class CARIBBEANUNTITLED_API UStaminaSystem : public UObject
{
    GENERATED_BODY()

public:
    void Tick(UWorld* World, float DeltaTime);
    void Cleanup();

    void DrainFromSprint(
        UStatsComponent* Stats,
        UCharacterMovementComponent* Movement,
        UControlComponent* Control,
        float DeltaTime,
        bool bIsMoving);

    void DrainFromJump(
        UStatsComponent* Stats,
        ACharacter* Character,
        float DeltaTime);

    void DrainFromLightAttack(
        UStatsComponent* Stats,
        AActor* Actor,
        float DeltaTime);

    void DrainFromHeavyAttack(
        UStatsComponent* Stats,
        AActor* Actor,
        float DeltaTime);

    /**
     * How quickly sprint speed eases back toward walking speed.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina|Sprint", meta=(ClampMin="0.0"))
    float SprintReturnInterpSpeed = 2.5f;

    /**
     * Stamina ratio above which the actor can hold full sprint speed.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina|Sprint", meta=(ClampMin="0.0", ClampMax="1.0"))
    float FullSprintThreshold = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina|Sprint", meta=(ClampMin="0.0", ClampMax="1.0"))
    float StrongSprintThreshold = 0.50f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina|Sprint", meta=(ClampMin="0.0", ClampMax="1.0"))
    float LightSprintThreshold = 0.25f;

private:
    float GetSprintSpeedForStamina(const UStatsComponent* Stats, float StaminaPercent) const;
    void ProcessActor(AActor* Actor, float DeltaTime);
};
