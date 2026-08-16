#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CameraSystem.generated.h"

class AActor;
class UWorld;

USTRUCT()
struct FCameraState
{
    GENERATED_BODY()

    FVector CurrentOffset = FVector::ZeroVector;
    FVector TargetOffset = FVector::ZeroVector;

    float CurrentArmLength = 0.0f;
    float TargetArmLength = 0.0f;
};

UCLASS()
class CARIBBEANUNTITLED_API UCameraSystem : public UObject
{
    GENERATED_BODY()

public:

    UCameraSystem();

    void Tick(UWorld* World, float DeltaTime);

    void SetUpdateInterval(int32 InTicks);

private:

    void ProcessActor(
        AActor* Actor,
        float DeltaTime
    );

    bool IsRunning(AActor* Actor) const;

    bool IsActionState(AActor* Actor) const;

    void CleanupInvalidActors();

private:

    int32 TickCounter = 0;

    int32 UpdateInterval = 2;

    TMap<
        TWeakObjectPtr<AActor>,
        FCameraState
    > CameraStates;


    // ============================================================
    // NORMAL
    // ============================================================

    UPROPERTY()
    FVector NormalOffset = FVector(0.0f, -120.0f, 50.0f);

    UPROPERTY()
    float NormalArmLength = 1000.0f;


    // ============================================================
    // RUNNING
    // ============================================================

    UPROPERTY()
    FVector RunningOffset = FVector(0.0f, 0.0f, 50.0f);

    UPROPERTY()
    float RunningArmLength = 1100.0f;


    // ============================================================
    // ACTION
    // ============================================================

    UPROPERTY()
    FVector ActionOffset = FVector(0.0f, -220.0f, 70.0f);

    UPROPERTY()
    float ActionArmLength = 1050.0f;


    // ============================================================
    // DAMPING
    // ============================================================

    UPROPERTY()
    float PositionDamping = 6.0f;

    UPROPERTY()
    float ArmLengthDamping = 5.0f;
};
