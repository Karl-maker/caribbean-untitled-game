#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MovementSystem.generated.h"

class AActor;

UCLASS()
class CARIBBEANUNTITLED_API UMovementSystem : public UObject
{
    GENERATED_BODY()

public:

    void Tick(float DeltaTime);

private:
    void ProcessActor(AActor* Actor, float DeltaTime);

    void ApplyLook(class ACharacter* Character, class UControlComponent* Control);

    FVector ResolveMoveDirection(class ACharacter* Character, class UControlComponent* Control) const;
    FVector ComputeIntentDirection(class ACharacter* Character, const FVector2D& MoveIntent) const;

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
    
    FVector ResolveExternalDirection(class ACharacter* Character, class UControlComponent* Control) const;
    
    void ApplyLook(class ACharacter* Character, class UControlComponent* Control, float DeltaTime);
    
};
