#include "StatsComponent.h"

UStatsComponent::UStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


// ============================================================
// BEGIN PLAY
// ============================================================

void UStatsComponent::BeginPlay()
{
    Super::BeginPlay();

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
    if (Amount <= 0.0f)
    {
        return true;
    }

    return Stamina >= Amount;
}


bool UStatsComponent::ConsumeStamina(float Amount)
{
    if (Amount <= 0.0f)
    {
        return true;
    }

    if (!HasStamina(Amount))
    {
        return false;
    }

    Stamina = FMath::Clamp(
        Stamina - Amount,
        0.0f,
        MaxStamina
    );

    return true;
}


void UStatsComponent::RestoreStamina(float Amount)
{
    if (Amount <= 0.0f)
    {
        return;
    }

    Stamina = FMath::Clamp(
        Stamina + Amount,
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
