#include "PerceptionSystem.h"
#include "../Components/PerceptionComponent.h"
#include "../Components/StatsComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "UObject/UObjectIterator.h"

UPerceptionSystem::UPerceptionSystem()
{
    // Pawns (characters) + WorldDynamic (typical default for movable
    // pickups like weapons/tools). See the comment on
    // PerceivableObjectTypes for swapping this to a dedicated
    // custom object channel instead.
    PerceivableObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    PerceivableObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
}

FIntVector UPerceptionSystem::LocationToCell(const FVector& Location) const
{
    return FIntVector(
        FMath::FloorToInt(Location.X / CellSize),
        FMath::FloorToInt(Location.Y / CellSize),
        FMath::FloorToInt(Location.Z / CellSize));
}

const TArray<TWeakObjectPtr<AActor>>& UPerceptionSystem::GetNearbyActors(
    const FVector& Location,
    float QueryRadius)
{
    UWorld* World = GetWorld();
    checkf(World, TEXT("UPerceptionSystem::GetNearbyActors called with no valid World"));

    const FIntVector CellCoord = LocationToCell(Location);
    FPerceptionCell& Cell = Cells.FindOrAdd(CellCoord);

    const double Now = World->GetTimeSeconds();
    const bool bStale = (Now - Cell.LastQueryTime) > CacheLifetime;
    const bool bRadiusGrew = QueryRadius > Cell.QueriedRadius;

    if (bStale || bRadiusGrew)
    {
        // Query around the CELL centre, not the caller's exact
        // location, so every perceiver in this cell asks the exact
        // same question and can share the exact same answer.
        const FVector CellCentre = (FVector(CellCoord) + FVector(0.5f)) * CellSize;
        const float Radius = QueryRadius + QueryPadding;

        TArray<AActor*> Overlapping;
        TArray<AActor*> IgnoreActors; // none - each perceiver filters itself out below

        UKismetSystemLibrary::SphereOverlapActors(
            World,
            CellCentre,
            Radius,
            PerceivableObjectTypes,
            /*ActorClassFilter=*/nullptr,
            IgnoreActors,
            Overlapping);

        Cell.Actors.Reset(Overlapping.Num());
        for (AActor* Actor : Overlapping)
        {
            if (IsValid(Actor))
            {
                Cell.Actors.Add(Actor);
            }
        }

        Cell.LastQueryTime = Now;
        Cell.QueriedRadius = Radius;
    }

    return Cell.Actors;
}

bool UPerceptionSystem::IsInSightCone(
    const FVector& EyeLocation,
    const FVector& Forward,
    const FVector& TargetLocation,
    float SightRange,
    float SightAngleDegrees,
    float& OutDistance) const
{
    const FVector ToTarget = TargetLocation - EyeLocation;
    OutDistance = ToTarget.Size();

    if (OutDistance > SightRange || OutDistance <= KINDA_SMALL_NUMBER)
    {
        return false;
    }

    const FVector Direction = ToTarget / OutDistance;
    const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(SightAngleDegrees * 0.5f));
    const float Dot = FVector::DotProduct(Forward, Direction);

    return Dot >= CosHalfAngle;
}

bool UPerceptionSystem::HasLineOfSight(
    UWorld* World,
    AActor* Owner,
    const FVector& EyeLocation,
    AActor* Target,
    ECollisionChannel TraceChannel) const
{
    // The occlusion check - separate from, and only run after, the
    // cone check. Only the perceiver and the target itself are
    // ignored, so any wall/crate/other actor blocking the space
    // between them registers a hit and correctly marks it unseen.
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PerceptionLOS), /*bTraceComplex=*/true);
    Params.AddIgnoredActor(Owner);
    Params.AddIgnoredActor(Target);

    FHitResult Hit;
    const bool bBlocked = World->LineTraceSingleByChannel(
        Hit,
        EyeLocation,
        Target->GetActorLocation(),
        TraceChannel,
        Params);

    return !bBlocked;
}

void UPerceptionSystem::Tick(UWorld* World, float DeltaTime)
{
    if (!World)
    {
        return;
    }

    // Every perception component in this world, each on its own
    // cadence. TObjectIterator keeps UCharacterPerceptionComponent
    // free of any registration bookkeeping - if this ever shows up
    // in a profile with a very large actor count, swap it for a
    // manual Register/Unregister list on this subsystem instead.
    for (TObjectIterator<UCharacterPerceptionComponent> It; It; ++It)
    {
        UCharacterPerceptionComponent* Perception = *It;
        if (!IsValid(Perception) || Perception->GetWorld() != World)
        {
            continue;
        }

        AActor* Owner = Perception->GetOwner();
        if (!Owner)
        {
            continue;
        }

        Perception->TimeSinceLastUpdate += DeltaTime;
        if (Perception->TimeSinceLastUpdate < Perception->PerceptionUpdateInterval)
        {
            continue; // not due yet - this is the "not every tick" throttle
        }
        Perception->TimeSinceLastUpdate = 0.0f;

        UStatsComponent* Stats = Owner->FindComponentByClass<UStatsComponent>();
        if (!Stats)
        {
            continue;
        }

        EvaluatePerceiver(World, Owner, Perception, Stats);
    }
}

void UPerceptionSystem::EvaluatePerceiver(
    UWorld* World,
    AActor* Owner,
    UCharacterPerceptionComponent* Perception,
    UStatsComponent* Stats)
{
    const float SightRange = Stats->GetSightRange();
    const float SightAngle = Stats->GetSightAngle();
    const FVector EyeLocation = Owner->GetActorLocation(); // swap for an eye socket if you add one
    const FVector Forward = Owner->GetActorForwardVector();

    // Shared/cached candidate list - potentially reused by several
    // nearby perceivers - but the cone/LOS check below is still
    // done per-perceiver against their own facing and stats.
    const TArray<TWeakObjectPtr<AActor>>& NearbyActors = GetNearbyActors(EyeLocation, SightRange);

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    const TArray<FPerceivedActor>& PreviousActors = Perception->GetPerceivedActors();

    TMap<AActor*, int32> OldIndexByActor;
    OldIndexByActor.Reserve(PreviousActors.Num());
    for (int32 Index = 0; Index < PreviousActors.Num(); ++Index)
    {
        if (AActor* PrevActor = PreviousActors[Index].Actor)
        {
            OldIndexByActor.Add(PrevActor, Index);
        }
    }

    TArray<FPerceivedActor> NewPerceived;
    NewPerceived.Reserve(PreviousActors.Num());

    TSet<AActor*> HandledThisPass;
    HandledThisPass.Reserve(NearbyActors.Num());

    for (const TWeakObjectPtr<AActor>& WeakActor : NearbyActors)
    {
        AActor* Actor = WeakActor.Get();
        if (!Actor || Actor == Owner)
        {
            continue;
        }

        HandledThisPass.Add(Actor);

        const int32* OldIndex = OldIndexByActor.Find(Actor);
        const FVector ActorLocation = Actor->GetActorLocation();

        // Already seeing this one and it barely moved - carry it
        // forward instead of redoing the angle/LOS work.
        if (OldIndex)
        {
            const FPerceivedActor& OldEntry = PreviousActors[*OldIndex];
            const float Moved = FVector::Dist(OldEntry.LastKnownLocation, ActorLocation);
            if (OldEntry.bVisible && Moved < Perception->MinMovementToRecheck)
            {
                FPerceivedActor Carried = OldEntry;
                Carried.TimeSinceLastSeen = 0.0f;
                NewPerceived.Add(Carried);
                continue;
            }
        }

        float Distance = 0.0f;
        const bool bInCone = IsInSightCone(EyeLocation, Forward, ActorLocation, SightRange, SightAngle, Distance);
        const bool bVisible = bInCone && HasLineOfSight(World, Owner, EyeLocation, Actor, Perception->SightTraceChannel);

        if (bVisible)
        {
            FPerceivedActor NewEntry;
            NewEntry.Actor = Actor;
            NewEntry.Distance = Distance;
            NewEntry.bVisible = true;
            NewEntry.LastKnownLocation = ActorLocation;
            NewEntry.TimeSinceLastSeen = 0.0f;
            NewPerceived.Add(NewEntry);
        }
        else if (OldIndex)
        {
            // Was perceived, isn't visible this pass - keep it
            // (last known location) unless it moved far or it's
            // been too long.
            FPerceivedActor Kept = PreviousActors[*OldIndex];
            Kept.bVisible = false;
            Kept.TimeSinceLastSeen += Perception->PerceptionUpdateInterval;

            const float MovedFromLastKnown = FVector::Dist(Kept.LastKnownLocation, ActorLocation);
            const bool bMovedTooFar = MovedFromLastKnown > Perception->ForgetDistance;
            const bool bTooOld = Kept.TimeSinceLastSeen > Perception->ForgetTime;

            if (!(bMovedTooFar || bTooOld))
            {
                NewPerceived.Add(Kept);
            }
        }
    }

    // Anything perceived before that wasn't even in this pass's
    // nearby list (cache miss / wandered out of query radius) still
    // gets the same grace period rather than vanishing instantly.
    for (int32 Index = 0; Index < PreviousActors.Num(); ++Index)
    {
        AActor* PrevActor = PreviousActors[Index].Actor;
        if (!PrevActor || HandledThisPass.Contains(PrevActor))
        {
            continue;
        }

        FPerceivedActor Kept = PreviousActors[Index];
        Kept.bVisible = false;
        Kept.TimeSinceLastSeen += Perception->PerceptionUpdateInterval;

        if (Kept.TimeSinceLastSeen <= Perception->ForgetTime)
        {
            NewPerceived.Add(Kept);
        }
    }

    // Player-specific fields, derived from the same result.
    bool bCanSeePlayer = false;
    AActor* PerceivedPlayer = nullptr;
    FVector LastKnownPlayerLocation = Perception->LastKnownPlayerLocation;

    if (PlayerPawn)
    {
        for (const FPerceivedActor& Entry : NewPerceived)
        {
            if (Entry.Actor == PlayerPawn)
            {
                PerceivedPlayer = PlayerPawn;
                LastKnownPlayerLocation = Entry.LastKnownLocation;
                bCanSeePlayer = Entry.bVisible;
                break;
            }
        }
    }

    Perception->SetPerceivedActors(NewPerceived);
    Perception->SetPlayerPerception(PerceivedPlayer, bCanSeePlayer, LastKnownPlayerLocation);

    // Threats intentionally untouched - left for another system.
}
