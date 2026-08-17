#include "CharacterBrainComponent.h"
#include "../Systems/BrainSystem.h"
#include "Engine/World.h"

UCharacterBrainComponent::UCharacterBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterBrainComponent::SetIntent(
    EBrainIntent NewIntent,
    float Effort)
{
    CurrentIntent.Intent = NewIntent;
    CurrentIntent.Effort = FMath::Clamp(Effort, 0.0f, 1.0f);
}

void UCharacterBrainComponent::SetFeeling(EBrainFeeling NewFeeling)
{
    CurrentIntent.Feeling = NewFeeling;
}

void UCharacterBrainComponent::SetTargetActor(AActor* Target)
{
    CurrentIntent.TargetActor = Target;
}

void UCharacterBrainComponent::SetTargetLocation(const FVector& Location)
{
    CurrentIntent.TargetLocation = Location;
}

void UCharacterBrainComponent::ClearIntent()
{
    CurrentIntent.Clear();
}

