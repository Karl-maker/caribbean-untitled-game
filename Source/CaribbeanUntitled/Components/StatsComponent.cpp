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

    Health = FMath::Clamp(
        Health,
        0.0f,
        MaxHealth
    );
}


// ============================================================
// MOVEMENT
// ============================================================

float UStatsComponent::GetWalkSpeed() const
{
    return WalkSpeed;
}


float UStatsComponent::GetSprintSpeed() const
{
    return SprintSpeed;
}


float UStatsComponent::GetCrouchSpeed() const
{
    return CrouchSpeed;
}


float UStatsComponent::GetSprintStaminaCostPerSecond() const
{
    return SprintStaminaCostPerSecond;
}


float UStatsComponent::GetJumpStrength() const
{
    return JumpStrength;
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
// HEALTH
// ============================================================

float UStatsComponent::GetHealth() const
{
    return Health;
}

float UStatsComponent::GetMaxHealth() const
{
    return MaxHealth;
}

float UStatsComponent::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f)
    {
        return 0.0f;
    }

    return Health / MaxHealth;
}

bool UStatsComponent::ApplyDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f || IsDead())
    {
        return false;
    }

    // Apply defense reduction
    float ReducedDamage = FMath::Max(0.0f, DamageAmount - Defense);
    
    Health = FMath::Clamp(
        Health - ReducedDamage,
        0.0f,
        MaxHealth
    );

    return true;
}

void UStatsComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f)
    {
        return;
    }

    Health = FMath::Clamp(
        Health + HealAmount,
        0.0f,
        MaxHealth
    );
}

void UStatsComponent::SetHealth(float NewHealth)
{
    Health = FMath::Clamp(
        NewHealth,
        0.0f,
        MaxHealth
    );
}

void UStatsComponent::RestoreFullHealth()
{
    Health = MaxHealth;
}

bool UStatsComponent::IsDead() const
{
    return Health <= 0.0f;
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


// ============================================================
// SIGHT
// ============================================================

float UStatsComponent::GetSightRange() const
{
    return SightRange;
}

float UStatsComponent::GetSightAngle() const
{
    return SightAngle;
}


// ============================================================
// COMBAT STATS
// ============================================================

float UStatsComponent::GetStrength() const
{
    return Strength;
}

float UStatsComponent::GetDefense() const
{
    return Defense;
}

float UStatsComponent::GetPoise() const
{
    return Poise;
}


// ============================================================
// BLOCK
// ============================================================

float UStatsComponent::GetBlockDamageReduction() const
{
    return BlockDamageReduction;
}

float UStatsComponent::GetBlockStaminaCost() const
{
    return BlockStaminaCost;
}

float UStatsComponent::GetBlockAngle() const
{
    return BlockAngle;
}


// ============================================================
// DODGE
// ============================================================

float UStatsComponent::GetDodgeDistance() const
{
    return DodgeDistance;
}

float UStatsComponent::GetDodgeDuration() const
{
    return DodgeDuration;
}

float UStatsComponent::GetDodgeStaminaCost() const
{
    return DodgeStaminaCost;
}

float UStatsComponent::GetDodgeCooldown() const
{
    return DodgeCooldown;
}

float UStatsComponent::GetDodgeInvulnerabilityDuration() const
{
    return DodgeInvulnerabilityDuration;
}
