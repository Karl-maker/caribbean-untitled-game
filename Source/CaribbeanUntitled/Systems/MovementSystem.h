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

    void Tick(UWorld* World, float DeltaTime);
    void Cleanup();
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
        ELocomotion PreviousState) const;

    UEventBusSubsystem* GetEventBus() const;

private:
    TMap<TWeakObjectPtr<AActor>, ELocomotion> PreviousLocomotionStates;
    TMap<TWeakObjectPtr<AActor>, FTimerHandle> LandingTimers;

    float LandingDuration = 0.7f;
    
    void ClearLandingState(TWeakObjectPtr<AActor> WeakActor);
    void ClearAllTimersForActor(AActor* Actor);
    void ProcessActor(AActor* Actor, float DeltaTime);

    FVector ResolveMoveDirection(class ACharacter* Character, class UControlComponent* Control) const;
    FVector ComputeIntentDirection(class ACharacter* Character, const FVector2D& MoveIntent) const;
    FVector ResolveExternalDirection(class ACharacter* Character, class UControlComponent* Control) const;

    void ApplySpeed(
        class UStatsComponent* Stats,
        class UCharacterMovementComponent* Movement,
        bool bWantsToSprint,
        bool bWantsToCrouch);

    void ApplyMovement(class ACharacter* Character, const FVector& MoveDirection);

    void ApplyCrouch(class ACharacter* Character, bool bWantsToCrouch);
    ELocomotion GetCurrentLocomotionState(AActor* Actor) const;
    
};
