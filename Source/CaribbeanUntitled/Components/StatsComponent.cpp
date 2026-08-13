//
//  StatsComponent.cpp
//  CaribbeanUntitled
//
//  Created by Karl-Johan Bailey on 8/12/26.
//

#include "StatsComponent.h"


UStatsComponent::UStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


// ============================================================
// LIFECYCLE
// ============================================================

void UStatsComponent::BeginPlay()
{
    Super::BeginPlay();

    // Make sure starting stamina never exceeds maximum stamina.
    Stamina = FMath::Clamp(
        Stamina,
        0.0f,
        MaxStamina
    );
}


// ============================================================
// STAMINA
// ============================================================

float UStatsComponent::GetStamina() const
{
    return Stamina;
}


float UStatsComponent::GetMaxStamina() const
{
    return MaxStamina;
}


float UStatsComponent::GetStaminaPercent() const
{
    if (MaxStamina <= 0.0f)
    {
        return 0.0f;
    }

    return Stamina / MaxStamina;
}


bool UStatsComponent::HasStamina(float Amount) const
{
    return Stamina >= FMath::Max(Amount, 0.0f);
}


bool UStatsComponent::ConsumeStamina(float Amount)
{
    Amount = FMath::Max(Amount, 0.0f);

    if (!HasStamina(Amount))
    {
        return false;
    }

    Stamina -= Amount;

    Stamina = FMath::Clamp(
        Stamina,
        0.0f,
        MaxStamina
    );

    return true;
}


void UStatsComponent::RestoreStamina(float Amount)
{
    Amount = FMath::Max(Amount, 0.0f);

    Stamina += Amount;

    Stamina = FMath::Clamp(
        Stamina,
        0.0f,
        MaxStamina
    );
}


void UStatsComponent::SetStamina(float NewStamina)
{
    Stamina = FMath::Clamp(
        NewStamina,
        0.0f,
        MaxStamina
    );
}


void UStatsComponent::RestoreFullStamina()
{
    Stamina = MaxStamina;
}


// ============================================================
// MOVEMENT
// ============================================================

float UStatsComponent::GetWalkSpeed() const
{
    return WalkSpeed;
}


float UStatsComponent::GetRunSpeed() const
{
    return RunSpeed;
}


float UStatsComponent::GetSprintSpeed() const
{
    return SprintSpeed;
}


float UStatsComponent::GetCrouchSpeed() const
{
    return CrouchSpeed;
}


// ============================================================
// DASH
// ============================================================

float UStatsComponent::GetDashSpeed() const
{
    return DashSpeed;
}


float UStatsComponent::GetDashDistance() const
{
    return DashDistance;
}


float UStatsComponent::GetDashDuration() const
{
    return DashDuration;
}
