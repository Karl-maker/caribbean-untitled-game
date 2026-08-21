// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"

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

void UCombatComponent::SetCanAttack(bool bNewCanAttack)
{
    bCanAttack = bNewCanAttack;
    
    if (!bCanAttack)
    {
        // Reset the requested action when attacks are disabled
        RequestedAction = ECombatAction::None;
    }
}
