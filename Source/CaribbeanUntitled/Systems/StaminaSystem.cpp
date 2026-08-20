//
//  StaminaSystem.cpp
//  CaribbeanUntitled (Mac)
//
//  Created by Family on 8/20/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//

#include "StaminaSystem.h"

#include "ActorRegistry.h"
#include "../Components/ControlComponent.h"
#include "../Components/StatsComponent.h"

#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UStaminaSystem::Tick(float DeltaTime)
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

    UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();
    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors =
        Registry->GetActorsWithComponent<UStatsComponent>();

    if (!Actors)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *Actors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            ProcessActor(Actor, DeltaTime);
        }
    }
}

void UStaminaSystem::Cleanup()
{
}

void UStaminaSystem::ProcessActor(AActor* Actor, float DeltaTime)
{
    if (!IsValid(Actor))
    {
        return;
    }

    UStatsComponent* Stats = Actor->FindComponentByClass<UStatsComponent>();
    UControlComponent* Control = Actor->FindComponentByClass<UControlComponent>();
    UCharacterMovementComponent* Movement = Actor->FindComponentByClass<UCharacterMovementComponent>();

    if (!Stats || !Control || !Movement)
    {
        return;
    }

    const bool bWantsToSprint = Control->WantsToSprint();
    const bool bIsMoving = Movement->Velocity.SizeSquared2D() > FMath::Square(10.0f);

    DrainFromSprint(Stats, Movement, Control, DeltaTime, bWantsToSprint && bIsMoving);
}

float UStaminaSystem::GetSprintSpeedForStamina(
    const UStatsComponent* Stats,
    float StaminaPercent) const
{
    if (!Stats)
    {
        return 0.0f;
    }

    const float WalkSpeed = Stats->GetWalkSpeed();
    const float SprintSpeed = Stats->GetSprintSpeed();

    if (StaminaPercent >= FullSprintThreshold)
    {
        return SprintSpeed;
    }

    if (StaminaPercent >= StrongSprintThreshold)
    {
        return FMath::Lerp(WalkSpeed, SprintSpeed, 0.80f);
    }

    if (StaminaPercent >= LightSprintThreshold)
    {
        return FMath::Lerp(WalkSpeed, SprintSpeed, 0.55f);
    }

    if (StaminaPercent > 0.05f)
    {
        return FMath::Lerp(WalkSpeed, SprintSpeed, 0.30f);
    }

    return WalkSpeed;
}

void UStaminaSystem::DrainFromSprint(
    UStatsComponent* Stats,
    UCharacterMovementComponent* Movement,
    UControlComponent* Control,
    float DeltaTime,
    bool bIsMoving)
{
    if (!Stats || !Movement || !Control)
    {
        return;
    }

    const float WalkSpeed = Stats->GetWalkSpeed();
    float TargetSpeed = WalkSpeed;

    if (Control->WantsToCrouch())
    {
        TargetSpeed = Stats->GetCrouchSpeed();
    }
    else if (Control->WantsToSprint() && bIsMoving)
    {
        const float Cost = Stats->GetSprintStaminaCostPerSecond() * DeltaTime;
        const bool bHadStamina = Stats->ConsumeStamina(Cost);

        if (bHadStamina && Stats->GetMaxStamina() > 0.0f)
        {
            const float StaminaAlpha = FMath::Clamp(Stats->GetStaminaPercent(), 0.0f, 1.0f);
            TargetSpeed = GetSprintSpeedForStamina(Stats, StaminaAlpha);
        }
        else
        {
            TargetSpeed = WalkSpeed;
        }
    }

    Movement->MaxWalkSpeed = FMath::FInterpTo(
        Movement->MaxWalkSpeed,
        TargetSpeed,
        DeltaTime,
        SprintReturnInterpSpeed
    );
}

void UStaminaSystem::DrainFromJump(
    UStatsComponent* Stats,
    ACharacter* Character,
    float DeltaTime)
{
    (void)Stats;
    (void)Character;
    (void)DeltaTime;
}

void UStaminaSystem::DrainFromLightAttack(
    UStatsComponent* Stats,
    AActor* Actor,
    float DeltaTime)
{
    (void)Stats;
    (void)Actor;
    (void)DeltaTime;
}

void UStaminaSystem::DrainFromHeavyAttack(
    UStatsComponent* Stats,
    AActor* Actor,
    float DeltaTime)
{
    (void)Stats;
    (void)Actor;
    (void)DeltaTime;
}
