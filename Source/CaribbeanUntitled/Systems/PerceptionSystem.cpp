#include "PerceptionSystem.h"
#include "ActorRegistry.h"
#include "../Components/PerceptionComponent.h"
#include "../Components/StatsComponent.h"

#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"


void UPerceptionSystem::Cleanup()
{
    Cells.Reset();
}

UPerceptionSystem::UPerceptionSystem()
{
    PerceivableObjectTypes.Add(
        UEngineTypes::ConvertToObjectType(ECC_Pawn));

    PerceivableObjectTypes.Add(
        UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

}

FIntVector UPerceptionSystem::LocationToCell(const FVector& Location) const
{
    return FIntVector(
        FMath::FloorToInt(Location.X / CellSize),
        FMath::FloorToInt(Location.Y / CellSize),
        FMath::FloorToInt(Location.Z / CellSize));
}

const TArray<TWeakObjectPtr<AActor>>& UPerceptionSystem::GetNearbyActors(
    UWorld* World,
    const FVector& Location,
    float QueryRadius)
{

    checkf(
        World,
        TEXT("UPerceptionSystem::GetNearbyActors called with no valid World")
    );

    const FIntVector CellCoord = LocationToCell(Location);
    FPerceptionCell& Cell = Cells.FindOrAdd(CellCoord);

    const double Now = World->GetTimeSeconds();

    const bool bStale =
        (Now - Cell.LastQueryTime) > CacheLifetime;

    const bool bRadiusGrew =
        QueryRadius > Cell.QueriedRadius;

    if (bStale || bRadiusGrew)
    {
        const FVector CellCentre =
            (FVector(CellCoord) + FVector(0.5f)) * CellSize;

        // The query is centered on the cell rather than this individual
        // perceiver. Cover the farthest point in the cell so actors near an
        // edge do not lose valid candidates.
        const float CellCornerAllowance =
            0.5f * CellSize * FMath::Sqrt(3.0f);

        const float Radius =
            QueryRadius + QueryPadding + CellCornerAllowance;

        TArray<AActor*> Overlapping;
        TArray<AActor*> IgnoreActors;

        UKismetSystemLibrary::SphereOverlapActors(
            World,
            CellCentre,
            Radius,
            PerceivableObjectTypes,
            nullptr,
            IgnoreActors,
            Overlapping
        );

        Cell.Actors.Reset(Overlapping.Num());

        for (AActor* Actor : Overlapping)
        {
            if (IsValid(Actor))
            {
                Cell.Actors.Add(Actor);
            }
        }

        Cell.LastQueryTime = Now;
        Cell.QueriedRadius = QueryRadius;


        for (const TWeakObjectPtr<AActor>& WeakActor : Cell.Actors)
        {
            if (AActor* Actor = WeakActor.Get())
            {
            }
        }
    }
    else
    {
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
    const FVector ToTarget =
        TargetLocation - EyeLocation;

    OutDistance = ToTarget.Size();

    if (OutDistance > SightRange ||
        OutDistance <= KINDA_SMALL_NUMBER)
    {
        return false;
    }

    const FVector Direction =
        ToTarget / OutDistance;

    const float CosHalfAngle =
        FMath::Cos(
            FMath::DegreesToRadians(
                SightAngleDegrees * 0.5f));

    const float Dot =
        FVector::DotProduct(
            Forward,
            Direction);

    return Dot >= CosHalfAngle;
}

bool UPerceptionSystem::HasLineOfSight(
    UWorld* World,
    AActor* Owner,
    const FVector& EyeLocation,
    AActor* Target,
    ECollisionChannel TraceChannel) const
{
    FCollisionQueryParams Params(
        SCENE_QUERY_STAT(PerceptionLOS),
        true
    );

    Params.AddIgnoredActor(Owner);
    Params.AddIgnoredActor(Target);

    FHitResult Hit;

    const bool bBlocked =
        World->LineTraceSingleByChannel(
            Hit,
            EyeLocation,
            Target->GetActorLocation(),
            TraceChannel,
            Params
        );

    if (bBlocked)
    {
    }

    return !bBlocked;
}

void UPerceptionSystem::Tick(
    UWorld* World,
    float DeltaTime)
{
    if (!World)
    {
        return;
    }

    UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();
    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* PerceptionActors =
        Registry->GetActorsWithComponent<UCharacterPerceptionComponent>();
    if (!PerceptionActors)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *PerceptionActors)
    {
        AActor* Owner = WeakActor.Get();
        if (!IsValid(Owner))
        {
            continue;
        }

        UCharacterPerceptionComponent* Perception =
            Owner->FindComponentByClass<UCharacterPerceptionComponent>();
        if (!IsValid(Perception))
        {
            continue;
        }

        Perception->TimeSinceLastUpdate += DeltaTime;

        if (Perception->TimeSinceLastUpdate <
            Perception->PerceptionUpdateInterval)
        {
            continue;
        }

        Perception->TimeSinceLastUpdate = 0.0f;

        UStatsComponent* Stats =
            Owner->FindComponentByClass<UStatsComponent>();

        if (!Stats)
        {

            continue;
        }

        EvaluatePerceiver(
            World,
            Owner,
            Perception,
            Stats
        );
    }

}

void UPerceptionSystem::EvaluatePerceiver(
    UWorld* World,
    AActor* Owner,
    UCharacterPerceptionComponent* Perception,
    UStatsComponent* Stats)
{
    // --- SAFETY CHECKS ---
    if (!IsValid(Owner) || !IsValid(Perception) || !IsValid(Stats))
    {
        return;
    }

    const float SightRange = Stats->GetSightRange();
    const float SightAngle = Stats->GetSightAngle();
    const FVector EyeLocation = Owner->GetActorLocation();
    const FVector Forward = Owner->GetActorForwardVector();


    const TArray<TWeakObjectPtr<AActor>>& NearbyActors =
        GetNearbyActors(World, EyeLocation, SightRange);


    // --- PROCESS PERCEIVED ACTORS ---
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

    int32 VisibleCount = 0;
    int32 OutsideConeCount = 0;
    int32 BlockedCount = 0;
    int32 CarriedCount = 0;
    int32 ForgottenCount = 0;

    for (const TWeakObjectPtr<AActor>& WeakActor : NearbyActors)
    {
        AActor* Actor = WeakActor.Get();

        // --- SELF-PERCEPTION CHECK (IMPROVED) ---
        if (!Actor || Actor == Owner)
        {
            continue;
        }

        HandledThisPass.Add(Actor);

        const int32* OldIndex = OldIndexByActor.Find(Actor);
        const FVector ActorLocation = Actor->GetActorLocation();

        /*
         * Cached visible actor.
         */
        if (OldIndex)
        {
            const FPerceivedActor& OldEntry = PreviousActors[*OldIndex];

            const float Moved = FVector::Dist(OldEntry.LastKnownLocation, ActorLocation);

            if (OldEntry.bVisible && Moved < Perception->MinMovementToRecheck)
            {
                FPerceivedActor Carried = OldEntry;
                Carried.TimeSinceLastSeen = 0.0f;
                NewPerceived.Add(Carried);

                ++CarriedCount;


                continue;
            }
        }

        float Distance = 0.0f;

        const bool bInCone = IsInSightCone(
            EyeLocation,
            Forward,
            ActorLocation,
            SightRange,
            SightAngle,
            Distance
        );

        if (!bInCone)
        {
            ++OutsideConeCount;

        }

        const bool bVisible = bInCone && HasLineOfSight(
            World,
            Owner,
            EyeLocation,
            Actor,
            Perception->SightTraceChannel
        );

        if (bVisible)
        {
            ++VisibleCount;

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
            ++BlockedCount;

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
            else
            {
                ++ForgottenCount;

            }
        }
        else
        {
            if (bInCone)
            {
                ++BlockedCount;

            }
        }
    }

    /*
     * Preserve remembered actors that are no longer inside
     * this pass's nearby cache.
     */
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
        else
        {
            ++ForgottenCount;

        }
    }

    Perception->SetPerceivedActors(NewPerceived);

    /*
     * Final summary for this perceiver.
     */

    /*
     * Explicit list of everything currently perceived.
     */
    for (const FPerceivedActor& Entry : NewPerceived)
    {
        if (AActor* Actor = Entry.Actor)
        {
        }
    }
}
