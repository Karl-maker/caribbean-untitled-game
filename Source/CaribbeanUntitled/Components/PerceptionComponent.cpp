#include "PerceptionComponent.h"

UCharacterPerceptionComponent::UCharacterPerceptionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterPerceptionComponent::Clear()
{
    PerceivedActors.Empty();
    Threats.Empty();

    PerceivedPlayer = nullptr;

    LastKnownPlayerLocation = FVector::ZeroVector;

    bCanSeePlayer = false;
    bUnderThreat = false;
}

void UCharacterPerceptionComponent::SetPerceivedActors(
    const TArray<FPerceivedActor>& Actors)
{
    PerceivedActors = Actors;
}

void UCharacterPerceptionComponent::SetThreats(
    const TArray<FPerceivedActor>& NewThreats)
{
    Threats = NewThreats;

    bUnderThreat = !Threats.IsEmpty();
}

void UCharacterPerceptionComponent::SetPlayerPerception(
    AActor* Player,
    bool bVisible,
    const FVector& LastKnownLocation)
{
    PerceivedPlayer = Player;
    bCanSeePlayer = bVisible;

    if (Player && bVisible)
    {
        LastKnownPlayerLocation = Player->GetActorLocation();
    }
    else
    {
        LastKnownPlayerLocation = LastKnownLocation;
    }
}
