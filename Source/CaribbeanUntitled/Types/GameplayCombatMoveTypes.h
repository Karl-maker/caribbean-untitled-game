#pragma once

#include "CoreMinimal.h"
#include "../Components/AnimationRegistryComponent.h"
#include "../Components/CombatVolumeComponent.h"
#include "GameplayCombatMoveTypes.generated.h"

USTRUCT(BlueprintType)
struct FGameplayCombatMove
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FName MoveId;

    UPROPERTY(EditAnywhere)
    EAnimationKey Animation = EAnimationKey::None;

    UPROPERTY(EditAnywhere)
    float AnticipationTime = 0.15f;

    UPROPERTY(EditAnywhere)
    float ActiveTime = 0.10f;

    UPROPERTY(EditAnywhere)
    float RecoveryTime = 0.30f;

    UPROPERTY(EditAnywhere)
    float Damage = 10.0f;
    
    UPROPERTY(EditAnywhere)
    float Stamina = 10.0f;
    
    // Combat volumes used by this move.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
    TArray<TObjectPtr<class UCombatVolumeComponent>> CombatVolumes;
    // Stat damage as well ...
};
