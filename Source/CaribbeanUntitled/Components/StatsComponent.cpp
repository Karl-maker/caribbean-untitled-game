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

    MaxStamina = 100.0f;
    Stamina = MaxStamina;

    StaminaRegenRate = 15.0f;
    StaminaRegenDelay = 1.0f;
}


void UStatsComponent::BeginPlay()
{
    Super::BeginPlay();

    Stamina = FMath::Clamp(
        Stamina,
        0.0f,
        MaxStamina
    );
}


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
    if (MaxStamina <= KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }

    return FMath::Clamp(
        Stamina / MaxStamina,
        0.0f,
        1.0f
    );
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

    if (Stamina < Amount)
    {
        return false;
    }

    Stamina -= Amount;

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
