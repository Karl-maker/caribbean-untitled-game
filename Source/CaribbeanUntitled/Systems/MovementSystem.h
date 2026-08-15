#pragma once

#include "CoreMinimal.h"
#include "../Enums/Locomotion.h"
#include "../EventBus/EventBusSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "../Components/ControlComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

#include "MovementSystem.generated.h"

class AActor;

UCLASS()
class CARIBBEANUNTITLED_API UMovementSystem : public UObject
{
    GENERATED_BODY()

public:

    void Tick(float DeltaTime);

    void DetermineAndPublishLocomotion(
        AActor* Actor,
        ACharacter* Character,
        UCharacterMovementComponent* Movement,
        UControlComponent* Control,
        const FVector& MoveDirection,
        bool bWantsToSprint,
        bool bWantsToCrouch);

    ELocomotion DetermineLocomotionState(
        ACharacter* Character,
        UCharacterMovementComponent* Movement,
        UControlComponent* Control,
        const FVector& MoveDirection,
        bool bWantsToSprint,
        bool bWantsToCrouch,
        ELocomotion PreviousState,
        bool bJumpInProgress) const;

    UEventBusSubsystem* GetEventBus() const;

    TSet<TWeakObjectPtr<AActor>> AscendingJumpers;
    bool IsJumpInProgress(AActor* Actor) const;
private:
    TMap<TWeakObjectPtr<AActor>, ELocomotion> PreviousLocomotionStates;

    TMap<TWeakObjectPtr<AActor>, FTimerHandle> PendingJumpTimers;
    
    void ProcessActor(AActor* Actor, float DeltaTime);
    void PerformDelayedJump(TWeakObjectPtr<AActor> WeakActor, float AscentGravityScale);
    void ApplyLook(class ACharacter* Character, class UControlComponent* Control, float DeltaTime);

    FVector ResolveMoveDirection(class ACharacter* Character, class UControlComponent* Control) const;
    FVector ComputeIntentDirection(class ACharacter* Character, const FVector2D& MoveIntent) const;
    FVector ResolveExternalDirection(class ACharacter* Character, class UControlComponent* Control) const;

    void ApplySpeed(
        class UStatsComponent* Stats,
        class UCharacterMovementComponent* Movement,
        float DeltaTime,
        bool bWantsToSprint,
        bool bWantsToCrouch,
        bool bIsMoving);

    void ApplyJump(
        class ACharacter* Character,
        class UStatsComponent* Stats,
        class UCharacterMovementComponent* Movement,
        class UControlComponent* Control);

    void ApplyMovement(class ACharacter* Character, const FVector& MoveDirection);

    void ApplyCrouch(class ACharacter* Character, bool bWantsToCrouch);
    ELocomotion GetCurrentLocomotionState(AActor* Actor) const;
};
