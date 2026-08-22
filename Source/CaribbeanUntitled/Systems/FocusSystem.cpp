//
// FocusSystem.cpp
// CaribbeanUntitled
//

#include "FocusSystem.h"

#include "../Components/ControlComponent.h"
#include "../Components/IKComponent.h"
#include "../Components/InteractiveComponent.h"
#include "../Components/StatsComponent.h"
#include "ActorRegistry.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
    bool IsFocusableActor(AActor* Actor)
    {
        return IsValid(Actor) &&
            (Actor->FindComponentByClass<UStatsComponent>() != nullptr ||
             Actor->FindComponentByClass<UInteractiveComponent>() != nullptr);
    }
}

TStatId UFocusSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UFocusSystem, STATGROUP_Tickables);
}

void UFocusSystem::Cleanup()
{
    Cells.Reset();
    States.Reset();
}

FIntVector UFocusSystem::LocationToCell(const FVector& Location) const
{
    const float CellSize = FMath::Max(1.0f, QueryPadding * 4.0f);
    return FIntVector(
        FMath::FloorToInt(Location.X / CellSize),
        FMath::FloorToInt(Location.Y / CellSize),
        FMath::FloorToInt(Location.Z / CellSize));
}

const TArray<TWeakObjectPtr<AActor>>& UFocusSystem::GetNearbyActors(
    UWorld* World,
    const FVector& Location,
    float QueryRadius)
{
    static TArray<TWeakObjectPtr<AActor>> Empty;

    if (!World)
    {
        return Empty;
    }

    const FIntVector CellCoord = LocationToCell(Location);
    FFocusCell& Cell = Cells.FindOrAdd(CellCoord);

    const double Now = World->GetTimeSeconds();
    const bool bStale = (Now - Cell.LastQueryTime) > FMath::Max(0.1f, UpdateInterval);
    const bool bRadiusGrew = QueryRadius > Cell.QueriedRadius;

    if (!bStale && !bRadiusGrew)
    {
        return Cell.Actors;
    }

    const FVector CellCentre = (FVector(CellCoord) + FVector(0.5f)) * FMath::Max(1.0f, QueryRadius);
    const float Radius = QueryRadius + QueryPadding;

    TArray<AActor*> Overlapping;
    TArray<AActor*> IgnoreActors;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

    UKismetSystemLibrary::SphereOverlapActors(
        World,
        CellCentre,
        Radius,
        ObjectTypes,
        nullptr,
        IgnoreActors,
        Overlapping
    );

    Cell.Actors.Reset(Overlapping.Num());
    for (AActor* Actor : Overlapping)
    {
        if (IsFocusableActor(Actor))
        {
            Cell.Actors.Add(Actor);
        }
    }

    Cell.LastQueryTime = Now;
    Cell.QueriedRadius = Radius;
    return Cell.Actors;
}

FVector UFocusSystem::ComputeDesiredLookDirection(
    AActor* Owner,
    AActor* Target,
    const FFocusState& State) const
{
    if (!IsValid(Owner) || !IsValid(Target))
    {
        return State.SmoothedLookDirection.IsNearlyZero()
            ? FVector::ForwardVector
            : State.SmoothedLookDirection;
    }

    FVector ToTarget = Target->GetActorLocation() - Owner->GetActorLocation();
    ToTarget.Z = 0.0f;
    return ToTarget.GetSafeNormal();
}

FVector UFocusSystem::ClampLookDirectionToHeadLimits(
    AActor* Owner,
    const FVector& DesiredLookDirection) const
{
    const FVector BaseForward = Owner ? Owner->GetActorForwardVector() : FVector::ForwardVector;
    const FRotator LookRot = DesiredLookDirection.Rotation();
    const FRotator BaseRot = BaseForward.Rotation();

    const float DeltaYaw = FMath::FindDeltaAngleDegrees(BaseRot.Yaw, LookRot.Yaw);
    const float DeltaPitch = FMath::Clamp(LookRot.Pitch - BaseRot.Pitch, -MaxHeadPitch, MaxHeadPitch);

    const float ClampedYaw = FMath::Clamp(DeltaYaw, -MaxHeadYaw, MaxHeadYaw);
    const FRotator ClampedRot(BaseRot.Pitch + DeltaPitch, BaseRot.Yaw + ClampedYaw, 0.0f);
    return ClampedRot.Vector().GetSafeNormal();
}

AActor* UFocusSystem::FindBestTarget(
    AActor* Owner,
    UWorld* World,
    UStatsComponent* Stats,
    const TArray<TWeakObjectPtr<AActor>>& NearbyActors) const
{
    if (!IsValid(Owner) || !World || !Stats)
    {
        return nullptr;
    }

    const FVector EyeLocation = Owner->GetActorLocation();
    const FVector Forward = Owner->GetActorForwardVector();
    const float SightRange = Stats->GetSightRange();
    const float SightAngle = Stats->GetSightAngle();
    const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(SightAngle * 0.5f));

    AActor* Best = nullptr;
    float BestScore = TNumericLimits<float>::Max();

    for (const TWeakObjectPtr<AActor>& WeakActor : NearbyActors)
    {
        AActor* Actor = WeakActor.Get();
        if (!IsFocusableActor(Actor) || Actor == Owner)
        {
            continue;
        }

        const FVector ToTarget = Actor->GetActorLocation() - EyeLocation;
        const float Distance = ToTarget.Size();
        if (Distance <= KINDA_SMALL_NUMBER || Distance > SightRange)
        {
            continue;
        }

        const FVector Direction = ToTarget / Distance;
        if (FVector::DotProduct(Forward, Direction) < CosHalfAngle)
        {
            continue;
        }

        const float Score = Distance;
        if (Score < BestScore)
        {
            BestScore = Score;
            Best = Actor;
        }
    }

    return Best;
}

void UFocusSystem::EvaluateActor(
    AActor* Actor,
    UControlComponent* Control,
    UStatsComponent* Stats,
    FFocusState& State,
    UWorld* World,
    float DeltaTime)
{
    if (!IsValid(Actor) || !Control || !Stats || !World)
    {
        return;
    }

    const FVector EyeLocation = Actor->GetActorLocation();
    const float SightRange = Stats->GetSightRange();
    const TArray<TWeakObjectPtr<AActor>>& NearbyActors = GetNearbyActors(World, EyeLocation, SightRange);
    AActor* Target = FindBestTarget(Actor, World, Stats, NearbyActors);
    State.CurrentTarget = Target;

    FVector Desired = State.SmoothedLookDirection;

    if (Target)
    {
        Desired = ComputeDesiredLookDirection(Actor, Target, State);
        Desired = ClampLookDirectionToHeadLimits(Actor, Desired);
    }

    const float InterpAlpha = FMath::Clamp(DeltaTime * BlendSpeed, 0.0f, 1.0f);
    State.SmoothedLookDirection = FMath::Lerp(State.SmoothedLookDirection, Desired, InterpAlpha).GetSafeNormal();

    if (!State.SmoothedLookDirection.IsNearlyZero())
    {
        Control->SetLookDirection(State.SmoothedLookDirection);
        if (Target)
        {
            Control->SetLookAtLocation(Target->GetActorLocation());

            if (UIKComponent* IK = Actor->FindComponentByClass<UIKComponent>())
            {
                FIKTargetData TargetData;
                TargetData.bEnabled = true;
                TargetData.TargetLocation = Target->GetActorLocation();
                TargetData.TargetDirection = State.SmoothedLookDirection;
                TargetData.Weight = 1.0f;
                TargetData.BlendSpeed = BlendSpeed;
                TargetData.BoneName = TEXT("Head");
                TargetData.ParentBoneName = TEXT("Neck");
                IK->SetTarget(TEXT("HeadLook"), TargetData);
            }
        }
    }
}

void UFocusSystem::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();
    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors = Registry->GetActorsWithComponent<UControlComponent>();
    if (!Actors)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *Actors)
    {
        AActor* Actor = WeakActor.Get();
        if (!IsValid(Actor))
        {
            continue;
        }

        UControlComponent* Control = Actor->FindComponentByClass<UControlComponent>();
        UStatsComponent* Stats = Actor->FindComponentByClass<UStatsComponent>();
        if (!Control || !Stats)
        {
            continue;
        }

        FFocusState& State = States.FindOrAdd(Actor);
        State.TimeUntilNextUpdate -= DeltaTime;
        if (State.TimeUntilNextUpdate > 0.0f)
        {
            continue;
        }

        State.TimeUntilNextUpdate = UpdateInterval;
        EvaluateActor(Actor, Control, Stats, State, World, DeltaTime);
    }
}
