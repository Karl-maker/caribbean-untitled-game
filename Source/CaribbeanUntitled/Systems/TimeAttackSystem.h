//
//  TimeAttackSystem.h
//  CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "../Components/CombatVolumeComponent.h"
#include "SetupAttackSystem.h"
#include "TimeAttackSystem.generated.h"

class UAttackStateComponent;
class UCombatComponent;
class UEventBusSubsystem;
class UActorRegistry;

UCLASS()
class CARIBBEANUNTITLED_API UTimeAttackSystem
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

private:
    struct FActiveAttackState
    {
        FGameplayCombatMove Move;
        bool bCombatVolumeActive = false;
        bool bRecoveryComplete = false;
    };

    TMap<TWeakObjectPtr<AActor>, FActiveAttackState> ActiveAttacks;

    UEventBusSubsystem* GetEventBus() const;
    UActorRegistry* GetActorRegistry() const;
    void SubscribeToEvents();
    void UnsubscribeFromEvents();
    void OnAttackStarted(const FAttackStartedPayload& Payload);
    void ProcessActor(AActor* Actor, FActiveAttackState& State, float DeltaTime);
    void SetMoveVolumesActive(const FGameplayCombatMove& Move, bool bActive) const;
};
