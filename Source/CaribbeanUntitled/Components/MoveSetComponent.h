// MoveSetComponent.h
#pragma once

#include "CoreMinimal.h"
#include "../Types/GameplayCombatMoveTypes.h"
#include "AnimationRegistryComponent.h" // Assuming this contains FCombatMove definition
#include "MoveSetComponent.generated.h"

UENUM(BlueprintType)
enum class EMoveSetTypeKey : uint8
{
    None                UMETA(DisplayName = "None"),
    LightAttack1        UMETA(DisplayName = "Light Attack 1"),
    LightAttack2        UMETA(DisplayName = "Light Attack 2"),
    LightAttack3        UMETA(DisplayName = "Light Attack 3"),
    HeavyAttack1        UMETA(DisplayName = "Heavy Attack 1"),
    HeavyAttack2        UMETA(DisplayName = "Heavy Attack 2"),
    HeavyAttack3        UMETA(DisplayName = "Heavy Attack 3"),
    SpecialAttack1      UMETA(DisplayName = "Special Attack 1"),
    SpecialAttack2      UMETA(DisplayName = "Special Attack 2"),
    DashAttack          UMETA(DisplayName = "Dash Attack"),
    JumpAttack          UMETA(DisplayName = "Jump Attack"),
    CounterAttack       UMETA(DisplayName = "Counter Attack")
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UMoveSetComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMoveSetComponent();

    UFUNCTION(BlueprintCallable, Category = "Move Set")
    FGameplayCombatMove GetMove(EMoveSetTypeKey Key) const;
    
    UFUNCTION(BlueprintCallable, Category = "Move Set")
    void AddMove(EMoveSetTypeKey Key, const FGameplayCombatMove& Move);

    UFUNCTION(BlueprintCallable, Category = "Move Set")
    bool RemoveMove(EMoveSetTypeKey Key);

    UFUNCTION(BlueprintCallable, Category = "Move Set")
    void ClearAllMoves();

    // Optional: Check if a move exists for a given key
    UFUNCTION(BlueprintCallable, Category = "Move Set")
    bool HasMove(EMoveSetTypeKey Key) const;

    // Optional: Get all available move keys
    UFUNCTION(BlueprintCallable, Category = "Move Set")
    TArray<EMoveSetTypeKey> GetAvailableMoveKeys() const;

private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Move Set", meta = (AllowPrivateAccess = "true"))
    TMap<EMoveSetTypeKey, FGameplayCombatMove> Moves;

    // Helper to check if key is valid
    bool IsKeyValid(EMoveSetTypeKey Key) const;
};
