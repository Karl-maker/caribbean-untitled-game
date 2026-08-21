// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "../EventBus/EventBusSubsystem.h"
#include "../Systems/SetupAttackSystem.h"

UCombatComponent::UCombatComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = false;

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
    // Only process the request if we can attack and the action is valid
    if (!bCanAttack)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatComponent: Cannot attack right now!"));
        return;
    }

    if (Action == ECombatAction::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatComponent: Invalid attack action requested!"));
        return;
    }

    // Set the requested action
    RequestedAction = Action;
    
    // Log the attack request
    FString ActionName = UEnum::GetValueAsString(Action);
    UE_LOG(LogTemp, Log, TEXT("CombatComponent: Attack requested: %s"), *ActionName);

    // Here you would typically trigger the actual attack logic
    // For example, you might call a delegate or notify the owner actor
    // OnAttackRequested.Broadcast(Action);
}

ECombatAction UCombatComponent::GetRequestedAction() const
{
    return RequestedAction;
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
    if (CurrentAttackAnimationKey == NewAnimationKey)
    {
        UE_LOG(
            LogTemp,
            Verbose,
            TEXT("CombatComponent: Attack animation key already %d on %s"),
            static_cast<int32>(NewAnimationKey),
            *GetNameSafe(GetOwner())
        );
        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("CombatComponent: Setting attack animation key to %d on %s"),
        static_cast<int32>(NewAnimationKey),
        *GetNameSafe(GetOwner())
    );

    CurrentAttackAnimationKey = NewAnimationKey;

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("CombatComponent: Authority triggering OnRep_CurrentAttackAnimationKey on %s"),
            *GetNameSafe(GetOwner())
        );
        OnRep_CurrentAttackAnimationKey();
    }
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCombatComponent, CurrentAttackAnimationKey);
}

void UCombatComponent::OnRep_CurrentAttackAnimationKey()
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
                    TEXT("CombatComponent: OnRep_CurrentAttackAnimationKey publishing FAttackStartedPayload for %s with key %d"),
                    *Owner->GetName(),
                    static_cast<int32>(CurrentAttackAnimationKey)
                );

                FAttackStartedPayload Payload;
                Payload.SourceActor = Owner;
                EventBus->Publish<FAttackStartedPayload>(Payload);
                return;
            }
        }
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("CombatComponent: OnRep_CurrentAttackAnimationKey could not publish for %s"),
        *GetNameSafe(GetOwner())
    );
}
