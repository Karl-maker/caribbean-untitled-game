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
    PerceivedPlayer = nullptr;
    LastKnownPlayerLocation = FVector::ZeroVector;
    bCanSeePlayer = false;
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

void UCharacterPerceptionComponent::SetPlayerPerception(AActor* Player, bool bVisible, const FVector& LastKnownLocation)
{
    PerceivedPlayer = Player;
    bCanSeePlayer = bVisible;
    LastKnownPlayerLocation = LastKnownLocation;
}
