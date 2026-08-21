// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackStateComponent.generated.h"

UENUM(BlueprintType)
enum class EAttackPhase : uint8
{
    None            UMETA(DisplayName = "None"),
    Anticipation    UMETA(DisplayName = "Anticipation"),
    Active          UMETA(DisplayName = "Active"),
    Recovery        UMETA(DisplayName = "Recovery")
};

// Delegates for attack phase changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackPhaseChanged, EAttackPhase, NewPhase, EAttackPhase, OldPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackIndexChanged, int32, NewIndex, int32, OldIndex);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UAttackStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAttackStateComponent();

    // Getters
    UFUNCTION(BlueprintCallable, Category = "Attack State")
    EAttackPhase GetPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    int32 GetAttackIndex() const;

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    float GetPhaseTime() const;

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    float GetPhaseTimeNormalized(float TotalPhaseDuration) const;

    // Setters
    UFUNCTION(BlueprintCallable, Category = "Attack State")
    void SetPhase(EAttackPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    void SetAttackIndex(int32 NewIndex);

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    void SetPhaseTime(float NewTime);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Attack State")
    bool IsAttacking() const;

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    bool IsInPhase(EAttackPhase PhaseToCheck) const;

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    void ResetAttackState();

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    void AdvancePhase();

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    void UpdatePhaseTime(float DeltaTime);

    // Phase transition helpers
    UFUNCTION(BlueprintCallable, Category = "Attack State")
    void StartAnticipation(int32 AttackID);

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    void StartActivePhase();

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    void StartRecoveryPhase();

    UFUNCTION(BlueprintCallable, Category = "Attack State")
    void ClearAttackState();

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Attack State")
    FOnAttackPhaseChanged OnAttackPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Attack State")
    FOnAttackIndexChanged OnAttackIndexChanged;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Current attack phase
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack State", meta = (AllowPrivateAccess = "true"))
    EAttackPhase Phase;

    // Current attack index (used to identify which attack is being performed)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack State", meta = (AllowPrivateAccess = "true"))
    int32 AttackIndex;

    // Time spent in current phase
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack State", meta = (AllowPrivateAccess = "true"))
    float PhaseTime;

    // Internal function to handle phase changes
    void InternalSetPhase(EAttackPhase NewPhase);
    void InternalSetAttackIndex(int32 NewIndex);
};
