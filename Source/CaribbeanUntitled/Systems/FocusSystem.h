//
// FocusSystem.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tickable.h"
#include "FocusSystem.generated.h"

class UActorRegistry;
class UControlComponent;
class UInteractiveComponent;
class UStatsComponent;

UCLASS()
class CARIBBEANUNTITLED_API UFocusSystem : public UObject
{
    GENERATED_BODY()

public:
    void Cleanup();
    virtual void Tick(float DeltaTime);
    TStatId GetStatId() const;
    bool IsTickable() const { return true; }

    UPROPERTY(EditAnywhere, Category = "Focus")
    float UpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Focus", meta = (ClampMin = "0.0"))
    float QueryPadding = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Focus", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float MaxHeadYaw = 70.0f;

    UPROPERTY(EditAnywhere, Category = "Focus", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float MaxHeadPitch = 35.0f;

    UPROPERTY(EditAnywhere, Category = "Focus", meta = (ClampMin = "0.0"))
    float BlendSpeed = 2.0f;

private:
    struct FFocusCell
    {
        TArray<TWeakObjectPtr<AActor>> Actors;
        double LastQueryTime = -1000.0;
        float QueriedRadius = 0.0f;
    };

    struct FFocusState
    {
        float TimeUntilNextUpdate = 0.0f;
        FVector SmoothedLookDirection = FVector::ForwardVector;
        TWeakObjectPtr<AActor> CurrentTarget;
    };

    TMap<FIntVector, FFocusCell> Cells;
    TMap<TWeakObjectPtr<AActor>, FFocusState> States;

    FIntVector LocationToCell(const FVector& Location) const;
    const TArray<TWeakObjectPtr<AActor>>& GetNearbyActors(UWorld* World, const FVector& Location, float QueryRadius);
    void EvaluateActor(AActor* Actor, UControlComponent* Control, UStatsComponent* Stats, FFocusState& State, UWorld* World, float DeltaTime);
    AActor* FindBestTarget(AActor* Owner, UWorld* World, UStatsComponent* Stats, const TArray<TWeakObjectPtr<AActor>>& NearbyActors) const;
    FVector ComputeDesiredLookDirection(AActor* Owner, AActor* Target, const FFocusState& State) const;
    FVector ClampLookDirectionToHeadLimits(AActor* Owner, const FVector& DesiredLookDirection) const;
};
