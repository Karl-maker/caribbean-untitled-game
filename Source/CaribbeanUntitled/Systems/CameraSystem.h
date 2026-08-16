// CameraSystem.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h" // ECollisionChannel
#include "CameraSystem.generated.h"

class AActor;
class ACharacter;
class UWorld;
class USpringArmComponent;
class UControlComponent;

USTRUCT()
struct FCameraState
{
    GENERATED_BODY()

    // Current camera values
    FVector CurrentOffset = FVector::ZeroVector;
    float CurrentArmLength = 0.0f;
    FRotator CurrentRotation = FRotator::ZeroRotator;

    // Target values for smooth interpolation
    FVector TargetOffset = FVector::ZeroVector;
    float TargetArmLength = 0.0f;
    FRotator TargetRotation = FRotator::ZeroRotator;

    // Orbit angles (for third-person camera)
    float CurrentYaw = 0.0f;
    float CurrentPitch = 0.0f;
    float TargetYaw = 0.0f;
    float TargetPitch = 0.0f;

    // How long since the last non-zero look input, used to drive the
    // "settle back behind the character" auto re-center.
    float TimeSinceLastLookInput = 0.0f;

    // Whether this state has been initialized (absolute transform set,
    // starting yaw/pitch seeded, etc).
    bool bInitialized = false;
};

UCLASS()
class CARIBBEANUNTITLED_API UCameraSystem : public UObject
{
    GENERATED_BODY()

public:
    UCameraSystem();

    void Tick(UWorld* World, float DeltaTime);

    void SetUpdateInterval(int32 InTicks);

    // Camera control functions
    void SetMouseLook(const FVector2D& LookInput);
    void SetCameraTarget(AActor* InTarget);

    // Runtime adjustment
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetNormalCamera(const FVector& Offset, float ArmLength);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetRunningCamera(const FVector& Offset, float ArmLength);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetActionCamera(const FVector& Offset, float ArmLength);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetDamping(float InPositionDamping, float InArmLengthDamping, float InRotationDamping);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetMouseSensitivity(float InSensitivity);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetLookLimits(float MinPitch, float MaxPitch);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetJumpSmoothTime(float InJumpSmoothTime);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetAutoRecenter(bool bEnabled, float Delay, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraCollision(bool bEnabled, float ProbeSize);

private:
    void ProcessActor(AActor* Actor, float DeltaTime);
    void UpdateCameraState(AActor* Actor, USpringArmComponent* CameraBoom, float DeltaTime);

    bool IsRunning(AActor* Actor) const;
    bool IsActionState(AActor* Actor, bool bIsRecentlyJumping) const;
    void CleanupInvalidActors();

    // Determine target camera values based on character state
    void DetermineTargetValues(AActor* Actor, FVector& OutOffset, float& OutArmLength, float DeltaTime);

    // Update camera orbit with mouse/control input
    void UpdateCameraOrbit(ACharacter* Character, UControlComponent* Control, FCameraState& State, float DeltaTime);

private:
    int32 TickCounter = 0;
    int32 UpdateInterval = 1;

    TMap<TWeakObjectPtr<AActor>, FCameraState> CameraStates;

    // Mouse/Input - manual override source. If left zero, the system reads
    // look input directly from each Actor's UControlComponent instead.
    FVector2D MouseLookInput = FVector2D::ZeroVector;
    float MouseSensitivity = 0.05f;

    // Look limits (in degrees)
    float MinPitch = -60.0f;
    float MaxPitch = 60.0f;

    // Current camera target
    TWeakObjectPtr<AActor> CameraTarget;

    // Camera presets
    UPROPERTY()
    FVector NormalOffset = FVector(0.0f, 120.0f, 70.0f);
    UPROPERTY()
    float NormalArmLength = 220.0f;

    UPROPERTY()
    FVector RunningOffset = FVector(0.0f, 0.0f, 50.0f);
    UPROPERTY()
    float RunningArmLength = 280.0f;

    UPROPERTY()
    FVector ActionOffset = FVector(0.0f, 80.0f, 45.0f);
    UPROPERTY()
    float ActionArmLength = 250.0f;

    // Progressive zoom
    UPROPERTY()
    float MinArmLength = 180.0f;
    UPROPERTY()
    float MaxArmLength = 350.0f;
    UPROPERTY()
    float SpeedZoomThreshold = 100.0f;
    UPROPERTY()
    float MaxSpeedForZoom = 800.0f;

    // Damping
    UPROPERTY()
    float PositionDamping = 10.0f;
    UPROPERTY()
    float ArmLengthDamping = 8.0f;
    UPROPERTY()
    float RotationDamping = 12.0f;

    // Jump smoothing
    UPROPERTY()
    float JumpSmoothTime = 0.3f;
    float LastJumpTime = -1.0f;

    // Auto re-center: after a short pause with no look input (while
    // moving), the camera gently settles back behind the character.
    UPROPERTY()
    bool bAutoRecenterEnabled = true;
    UPROPERTY()
    float AutoRecenterDelay = 0.6f;
    UPROPERTY()
    float RecenterSpeed = 2.0f;

    // Camera collision (spring arm probe) so the camera doesn't clip
    // through walls/geometry - essential for an AAA feel.
    UPROPERTY()
    bool bCameraCollisionEnabled = true;
    UPROPERTY()
    float CameraProbeSize = 12.0f;
};
