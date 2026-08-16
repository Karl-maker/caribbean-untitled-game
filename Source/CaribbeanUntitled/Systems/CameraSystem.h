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
    
    // For smooth transitions
    float CurrentOffsetY = 0.0f;
    float TargetOffsetY = 0.0f;
};

UCLASS()
class CARIBBEANUNTITLED_API UCameraSystem : public UObject
{
    GENERATED_BODY()

public:

    UCameraSystem();

    void Tick(UWorld* World, float DeltaTime);

    void SetUpdateInterval(int32 InTicks);

    // Allow runtime adjustment of camera properties
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetNormalCamera(const FVector& Offset, float ArmLength);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetRunningCamera(const FVector& Offset, float ArmLength);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetActionCamera(const FVector& Offset, float ArmLength);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetDamping(float InPositionDamping, float InArmLengthDamping);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetJumpSmoothTime(float InJumpSmoothTime);

private:

    void ProcessActor(
        AActor* Actor,
        float DeltaTime
    );

    bool IsRunning(AActor* Actor) const;

    bool IsActionState(
        AActor* Actor,
        bool bIsRecentlyJumping
    ) const;

    void CleanupInvalidActors();

private:

    int32 TickCounter = 0;

    int32 UpdateInterval = 1;  // Default to every frame

    TMap<
        TWeakObjectPtr<AActor>,
        FCameraState
    > CameraStates;


    // ============================================================
    // NORMAL (Walking/Idle) - Right shoulder GOW style
    // ============================================================

    UPROPERTY()
    FVector NormalOffset = FVector(0.0f, 120.0f, 70.0f);  // Right offset for GOW style

    UPROPERTY()
    float NormalArmLength = 220.0f;  // Closer camera


    // ============================================================
    // RUNNING (Sprinting)
    // ============================================================

    UPROPERTY()
    FVector RunningOffset = FVector(0.0f, 0.0f, 50.0f);  // Centered when running

    UPROPERTY()
    float RunningArmLength = 280.0f;  // Closer when running


    // ============================================================
    // ACTION (Combat)
    // ============================================================

    UPROPERTY()
    FVector ActionOffset = FVector(0.0f, 80.0f, 45.0f);  // Right offset for combat

    UPROPERTY()
    float ActionArmLength = 250.0f;  // Closer for combat


    // ============================================================
    // PROGRESSIVE ZOOM (GTA Style)
    // ============================================================

    UPROPERTY()
    float MinArmLength = 180.0f;  // Minimum zoom distance (closer)

    UPROPERTY()
    float MaxArmLength = 350.0f;  // Maximum zoom distance

    UPROPERTY()
    float SpeedZoomThreshold = 100.0f;  // Speed to start zooming out

    UPROPERTY()
    float MaxSpeedForZoom = 800.0f;  // Speed for max zoom


    // ============================================================
    // DAMPING
    // ============================================================

    UPROPERTY()
    float PositionDamping = 10.0f;  // Slower for smoother transitions

    UPROPERTY()
    float ArmLengthDamping = 8.0f;

    UPROPERTY()
    float OffsetDamping = 6.0f;  // Separate damping for offset transitions


    // ============================================================
    // JUMP SMOOTHING
    // ============================================================

    /**
     * How long the camera stays in jump state after landing (seconds)
     * Prevents rapid camera flipping on quick jumps
     */
    UPROPERTY()
    float JumpSmoothTime = 0.3f;

    /**
     * Tracks how long since the last jump was detected
     * Used with JumpSmoothTime for hysteresis
     */
    float LastJumpTime = -1.0f;
};
