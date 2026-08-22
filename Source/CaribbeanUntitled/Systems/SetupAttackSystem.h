//
//  SetupAttackSystem.h
//  CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "../Components/CombatComponent.h"
#include "../Components/MoveSetComponent.h"
#include "../Types/GameplayCombatMoveTypes.h"
#include "SetupAttackSystem.generated.h"

class UActorRegistry;
class UAttackStateComponent;
class UEventBusSubsystem;

USTRUCT(BlueprintType)
struct CARIBBEANUNTITLED_API FAttackStartedPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TObjectPtr<AActor> SourceActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombatAction CombatAction = ECombatAction::None;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    EMoveSetTypeKey MoveKey = EMoveSetTypeKey::None;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FGameplayCombatMove Move;
};

UCLASS()
class CARIBBEANUNTITLED_API USetupAttackSystem
    : public UWorldSubsystem
    , public FTickableGameObject
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return true; }

    void RequestAttackForActor(AActor* Actor, ECombatAction RequestedAction);

private:
    UActorRegistry* GetActorRegistry() const;
    UEventBusSubsystem* GetEventBus() const;
    void ProcessActor(AActor* Actor);
    void StartAttack(
        AActor* Actor,
        UCombatComponent* Combat,
        UMoveSetComponent* MoveSet,
        UAttackStateComponent* AttackState,
        ECombatAction RequestedAction
    );
    bool ResolveMoveKey(ECombatAction Action, EMoveSetTypeKey& OutMoveKey) const;
};
