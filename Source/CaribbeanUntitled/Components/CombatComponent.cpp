// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "../EventBus/EventBusSubsystem.h"
#include "../Systems/SetupAttackSystem.h"
#include "Engine/World.h"

UCombatComponent::UCombatComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    // Initialize variables
    RequestedAction = ECombatAction::None;
    bCanAttack = true;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    // Any initialization that needs to happen when the component is first created
}

bool UCombatComponent::CanAttack() const
{
    return bCanAttack;
}

void UCombatComponent::RequestAttack(ECombatAction Action)
{
    UE_LOG(
        LogTemp,
        Log,
        TEXT("[CombatComponent]: RequestAttack called on %s | Action=%d | CanAttack=%d"),
        *GetNameSafe(GetOwner()),
        static_cast<int32>(Action),
        bCanAttack
    );

    // Only process the request if we can attack and the action is valid
    if (!bCanAttack)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatComponent]: Cannot attack right now!"));
        return;
    }

    if (Action == ECombatAction::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatComponent]: Invalid attack action requested!"));
        return;
    }

    // Set the requested action
    RequestedAction = Action;
    
    // Log the attack request
    FString ActionName = UEnum::GetValueAsString(Action);
    UE_LOG(LogTemp, Log, TEXT("[CombatComponent]: Attack requested: %s"), *ActionName);

    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = GetWorld())
        {
            if (USetupAttackSystem* SetupAttackSystem =
                World->GetSubsystem<USetupAttackSystem>())
            {
                UE_LOG(
                    LogTemp,
                    Log,
                    TEXT("[CombatComponent]: Forwarding attack request immediately to SetupAttackSystem for %s"),
                    *Owner->GetName()
                );
                SetupAttackSystem->RequestAttackForActor(
                    Owner,
                    Action
                );
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[CombatComponent]: SetupAttackSystem unavailable for %s"), *Owner->GetName());
            }
        }
    }
}

ECombatAction UCombatComponent::GetRequestedAction() const
{
    return RequestedAction;
}

void UCombatComponent::ClearRequestedAction()
{
    RequestedAction = ECombatAction::None;
}

EAnimationKey UCombatComponent::GetCurrentAttackAnimationKey() const
{
    return CurrentAttackAnimationKey;
}

void UCombatComponent::SetCanAttack(bool bNewCanAttack)
{
    bCanAttack = bNewCanAttack;
    
    if (!bCanAttack)
    {
        // Reset the requested action when attacks are disabled
        RequestedAction = ECombatAction::None;
    }
}

void UCombatComponent::SetCurrentAttackAnimationKey(EAnimationKey NewAnimationKey)
{
    UE_LOG(
        LogTemp,
        Log,
        TEXT("[CombatComponent]: Setting attack animation key to %d on %s"),
        static_cast<int32>(NewAnimationKey),
        *GetNameSafe(GetOwner())
    );

    CurrentAttackAnimationKey = NewAnimationKey;
}

void UCombatComponent::TriggerAttackAnimationReplication()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        ++AttackAnimationTrigger;
        UE_LOG(
            LogTemp,
            Log,
            TEXT("[CombatComponent]: Triggering attack animation replication %d on %s"),
            AttackAnimationTrigger,
            *GetNameSafe(GetOwner())
        );
        OnRep_AttackAnimationTrigger();
    }
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCombatComponent, CurrentAttackAnimationKey);
    DOREPLIFETIME(UCombatComponent, AttackAnimationTrigger);
}

void UCombatComponent::OnRep_AttackAnimationTrigger()
{
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = GetWorld())
        {
            if (UEventBusSubsystem* EventBus = World->GetSubsystem<UEventBusSubsystem>())
            {
                UE_LOG(
                    LogTemp,
                    Log,
                    TEXT("[CombatComponent]: OnRep_AttackAnimationTrigger publishing FAttackStartedPayload for %s with key %d (trigger=%d)"),
                    *Owner->GetName(),
                    static_cast<int32>(CurrentAttackAnimationKey),
                    AttackAnimationTrigger
                );

                FAttackStartedPayload Payload;
                Payload.SourceActor = Owner;
                Payload.Move.Animation = CurrentAttackAnimationKey;
                EventBus->Publish<FAttackStartedPayload>(Payload);
                return;
            }
        }
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[CombatComponent]: OnRep_AttackAnimationTrigger could not publish for %s"),
        *GetNameSafe(GetOwner())
    );
}
