// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Components/AnimationRegistryComponent.h"
#include "CombatComponent.generated.h"

UENUM(BlueprintType)
enum class ECombatAction : uint8
{
    None            UMETA(DisplayName = "None"),
    LightAttack     UMETA(DisplayName = "Light Attack"),
    HeavyAttack     UMETA(DisplayName = "Heavy Attack")
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatComponent();

    // Check if the component can currently perform an attack
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    // Request an attack action
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RequestAttack(ECombatAction Action);

    // Get the currently requested action
    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombatAction GetRequestedAction() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    EAnimationKey GetCurrentAttackAnimationKey() const;

    // Set whether the component can attack
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCanAttack(bool bNewCanAttack);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCurrentAttackAnimationKey(EAnimationKey NewAnimationKey);

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    // The currently requested combat action
    UPROPERTY(VisibleAnywhere, Category = "Combat")
    ECombatAction RequestedAction = ECombatAction::None;

    UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_CurrentAttackAnimationKey, Category = "Combat")
    EAnimationKey CurrentAttackAnimationKey = EAnimationKey::None;

    // Flag indicating if the component can perform attacks
    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bCanAttack = true;

    UFUNCTION()
    void OnRep_CurrentAttackAnimationKey();
};
