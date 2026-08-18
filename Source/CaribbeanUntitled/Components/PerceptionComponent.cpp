#include "PerceptionComponent.h"

UCharacterPerceptionComponent::UCharacterPerceptionComponent()
{
    // No component tick, no per-frame work of any kind - this is
    // pure data. UPerceptionSystem does all the evaluation and
    // writes results here through the setters below.
    PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterPerceptionComponent::Clear()
{
    PerceivedActors.Reset();
    Threats.Reset();
    bUnderThreat = false;
}

void UCharacterPerceptionComponent::SetPerceivedActors(const TArray<FPerceivedActor>& Actors)
{
    PerceivedActors = Actors;
}

void UCharacterPerceptionComponent::SetThreats(const TArray<FPerceivedActor>& NewThreats)
{
    Threats = NewThreats;
}
