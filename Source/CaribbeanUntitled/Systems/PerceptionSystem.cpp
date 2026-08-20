#include "PerceptionSystem.h"
#include "../Components/PerceptionComponent.h"
#include "../Components/StatsComponent.h"

#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "UObject/UObjectIterator.h"


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

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Perception] SYSTEM CREATED | ObjectTypes=%d | CellSize=%.1f | CacheLifetime=%.2f"),
        PerceivableObjectTypes.Num(),
        CellSize,
        CacheLifetime
    );
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

        const float Radius =
            QueryRadius + QueryPadding;

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
        Cell.QueriedRadius = Radius;

        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "[Perception] CACHE REFRESH | "
                "Cell=(%d,%d,%d) | "
                "QueryRadius=%.1f | "
                "ActorsFound=%d"
            ),
            CellCoord.X,
            CellCoord.Y,
            CellCoord.Z,
            Radius,
            Cell.Actors.Num()
        );

        for (const TWeakObjectPtr<AActor>& WeakActor : Cell.Actors)
        {
            if (AActor* Actor = WeakActor.Get())
            {
                UE_LOG(
                    LogTemp,
                    Verbose,
                    TEXT(
                        "[Perception] CACHE ACTOR | "
                        "Cell=(%d,%d,%d) | Actor=%s"
                    ),
                    CellCoord.X,
                    CellCoord.Y,
                    CellCoord.Z,
                    *Actor->GetName()
                );
            }
        }
    }
    else
    {
        UE_LOG(
            LogTemp,
            VeryVerbose,
            TEXT(
                "[Perception] CACHE HIT | "
                "Cell=(%d,%d,%d) | CachedActors=%d | "
                "QueriedRadius=%.1f | RequestedRadius=%.1f"
            ),
            CellCoord.X,
            CellCoord.Y,
            CellCoord.Z,
            Cell.Actors.Num(),
            Cell.QueriedRadius,
            QueryRadius
        );
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
        UE_LOG(
            LogTemp,
            VeryVerbose,
            TEXT(
                "[Perception] LOS BLOCKED | "
                "Owner=%s | Target=%s | Blocker=%s"
            ),
            *Owner->GetName(),
            *Target->GetName(),
            Hit.GetActor()
                ? *Hit.GetActor()->GetName()
                : TEXT("Unknown")
        );
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

    int32 ComponentsFound = 0;
    int32 ComponentsEvaluated = 0;

    for (TObjectIterator<UCharacterPerceptionComponent> It;
         It;
         ++It)
    {
        UCharacterPerceptionComponent* Perception = *It;

        if (!IsValid(Perception) ||
            Perception->GetWorld() != World)
        {
            continue;
        }

        ++ComponentsFound;

        AActor* Owner =
            Perception->GetOwner();

        if (!Owner)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "[Perception] COMPONENT HAS NO OWNER | "
                    "Component=%s"
                ),
                *Perception->GetName()
            );

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
            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "[Perception] NO STATS | "
                    "Owner=%s | Perception skipped"
                ),
                *Owner->GetName()
            );

            continue;
        }

        ++ComponentsEvaluated;

        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "[Perception] EVALUATE | "
                "Owner=%s | Interval=%.2f"
            ),
            *Owner->GetName(),
            Perception->PerceptionUpdateInterval
        );

        EvaluatePerceiver(
            World,
            Owner,
            Perception,
            Stats
        );
    }

    UE_LOG(
        LogTemp,
        VeryVerbose,
        TEXT(
            "[Perception] TICK | "
            "ComponentsFound=%d | Evaluated=%d"
        ),
        ComponentsFound,
        ComponentsEvaluated
    );
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
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Perception] EVALUATE SKIPPED | Invalid components")
        );
        return;
    }

    const float SightRange = Stats->GetSightRange();
    const float SightAngle = Stats->GetSightAngle();
    const FVector EyeLocation = Owner->GetActorLocation();
    const FVector Forward = Owner->GetActorForwardVector();

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Perception] SETTINGS | Owner=%s | Range=%.1f | Angle=%.1f"),
        *Owner->GetName(),
        SightRange,
        SightAngle
    );

    const TArray<TWeakObjectPtr<AActor>>& NearbyActors =
        GetNearbyActors(World, EyeLocation, SightRange);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Perception] NEARBY | Owner=%s | Candidates=%d"),
        *Owner->GetName(),
        NearbyActors.Num()
    );

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

                UE_LOG(
                    LogTemp,
                    Verbose,
                    TEXT("[Perception] CARRIED | Owner=%s | Target=%s | Moved=%.1f"),
                    *Owner->GetName(),
                    *Actor->GetName(),
                    Moved
                );

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

            UE_LOG(
                LogTemp,
                Verbose,
                TEXT("[Perception] NOT SEEN | Owner=%s | Target=%s | Reason=OutsideCone | Distance=%.1f"),
                *Owner->GetName(),
                *Actor->GetName(),
                Distance
            );
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

            UE_LOG(
                LogTemp,
                Log,
                TEXT("[Perception] PERCEIVED | Owner=%s | Target=%s | Distance=%.1f | Visible=true"),
                *Owner->GetName(),
                *Actor->GetName(),
                Distance
            );
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

                UE_LOG(
                    LogTemp,
                    Verbose,
                    TEXT("[Perception] MEMORY | Owner=%s | Target=%s | Visible=false | TimeSinceSeen=%.2f"),
                    *Owner->GetName(),
                    *Actor->GetName(),
                    Kept.TimeSinceLastSeen
                );
            }
            else
            {
                ++ForgottenCount;

                UE_LOG(
                    LogTemp,
                    Log,
                    TEXT("[Perception] FORGOTTEN | Owner=%s | Target=%s | MovedTooFar=%s | TooOld=%s"),
                    *Owner->GetName(),
                    *Actor->GetName(),
                    bMovedTooFar ? TEXT("true") : TEXT("false"),
                    bTooOld ? TEXT("true") : TEXT("false")
                );
            }
        }
        else
        {
            if (bInCone)
            {
                ++BlockedCount;

                UE_LOG(
                    LogTemp,
                    Verbose,
                    TEXT("[Perception] NOT SEEN | Owner=%s | Target=%s | Reason=BlockedByLOS"),
                    *Owner->GetName(),
                    *Actor->GetName()
                );
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

            UE_LOG(
                LogTemp,
                Verbose,
                TEXT("[Perception] MEMORY OUTSIDE CACHE | Owner=%s | Target=%s | TimeSinceSeen=%.2f"),
                *Owner->GetName(),
                *PrevActor->GetName(),
                Kept.TimeSinceLastSeen
            );
        }
        else
        {
            ++ForgottenCount;

            UE_LOG(
                LogTemp,
                Log,
                TEXT("[Perception] FORGOTTEN OUTSIDE CACHE | Owner=%s | Target=%s"),
                *Owner->GetName(),
                *PrevActor->GetName()
            );
        }
    }

    Perception->SetPerceivedActors(NewPerceived);

    /*
     * Final summary for this perceiver.
     */
    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Perception] RESULT | Owner=%s | Candidates=%d | Visible=%d | OutsideCone=%d | Blocked=%d | Carried=%d | Forgotten=%d | PerceivedTotal=%d"),
        *Owner->GetName(),
        NearbyActors.Num(),
        VisibleCount,
        OutsideConeCount,
        BlockedCount,
        CarriedCount,
        ForgottenCount,
        NewPerceived.Num()
    );

    /*
     * Explicit list of everything currently perceived.
     */
    for (const FPerceivedActor& Entry : NewPerceived)
    {
        if (AActor* Actor = Entry.Actor)
        {
            UE_LOG(
                LogTemp,
                Log,
                TEXT("[Perception] CURRENTLY PERCEIVED | Owner=%s | Target=%s | Visible=%s | Distance=%.1f | LastKnown=(%.1f, %.1f, %.1f)"),
                *Owner->GetName(),
                *Actor->GetName(),
                Entry.bVisible ? TEXT("true") : TEXT("false"),
                Entry.Distance,
                Entry.LastKnownLocation.X,
                Entry.LastKnownLocation.Y,
                Entry.LastKnownLocation.Z
            );
        }
    }
}
