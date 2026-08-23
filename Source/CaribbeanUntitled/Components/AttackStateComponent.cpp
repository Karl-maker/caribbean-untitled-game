// Fill out your copyright notice in the Description page of Project Settings.

#include "AttackStateComponent.h"

UAttackStateComponent::UAttackStateComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize state
    Phase = EAttackPhase::None;
    AttackIndex = -1;
    PhaseTime = 0.0f;
}

void UAttackStateComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UAttackStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Only update phase time if we're in a valid attack phase
    if (Phase != EAttackPhase::None)
    {
        PhaseTime += DeltaTime;
    }
}

// Getters
EAttackPhase UAttackStateComponent::GetPhase() const
{
    return Phase;
}

int32 UAttackStateComponent::GetAttackIndex() const
{
    return AttackIndex;
}

float UAttackStateComponent::GetPhaseTime() const
{
    return PhaseTime;
}

float UAttackStateComponent::GetPhaseTimeNormalized(float TotalPhaseDuration) const
{
    if (TotalPhaseDuration <= 0.0f)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(PhaseTime / TotalPhaseDuration, 0.0f, 1.0f);
}

// Setters
void UAttackStateComponent::SetPhase(EAttackPhase NewPhase)
{
    InternalSetPhase(NewPhase);
}

void UAttackStateComponent::SetAttackIndex(int32 NewIndex)
{
    InternalSetAttackIndex(NewIndex);
}

void UAttackStateComponent::SetPhaseTime(float NewTime)
{
    PhaseTime = FMath::Max(0.0f, NewTime);
}

// Utility functions
bool UAttackStateComponent::IsAttacking() const
{
    return Phase != EAttackPhase::None;
}

bool UAttackStateComponent::IsInPhase(EAttackPhase PhaseToCheck) const
{
    return Phase == PhaseToCheck;
}

void UAttackStateComponent::ResetAttackState()
{
    EAttackPhase OldPhase = Phase;
    int32 OldIndex = AttackIndex;
    
    Phase = EAttackPhase::None;
    AttackIndex = -1;
    PhaseTime = 0.0f;
    CurrentMoveKey = EMoveSetTypeKey::None;
    CurrentMoveId = NAME_None;
    
    // Broadcast changes
    if (OldPhase != Phase)
    {
        OnAttackPhaseChanged.Broadcast(Phase, OldPhase);
    }
    if (OldIndex != AttackIndex)
    {
        OnAttackIndexChanged.Broadcast(AttackIndex, OldIndex);
    }
}

void UAttackStateComponent::AdvancePhase()
{
    switch (Phase)
    {
        case EAttackPhase::Anticipation:
            SetPhase(EAttackPhase::Active);
            break;
            
        case EAttackPhase::Active:
            SetPhase(EAttackPhase::Recovery);
            break;
            
        case EAttackPhase::Recovery:
            ResetAttackState();
            break;
            
        default:
            break;
    }
}

void UAttackStateComponent::UpdatePhaseTime(float DeltaTime)
{
    if (Phase != EAttackPhase::None)
    {
        PhaseTime += DeltaTime;
    }
}

// Phase transition helpers
void UAttackStateComponent::StartAnticipation(int32 AttackID)
{
    InternalSetAttackIndex(AttackID);
    InternalSetPhase(EAttackPhase::Anticipation);
    PhaseTime = 0.0f;
    CurrentMoveKey = EMoveSetTypeKey::None;
    CurrentMoveId = NAME_None;
}

void UAttackStateComponent::StartAnticipationWithMove(int32 AttackID, EMoveSetTypeKey InMoveKey, FName InMoveId)
{
    InternalSetAttackIndex(AttackID);
    InternalSetPhase(EAttackPhase::Anticipation);
    PhaseTime = 0.0f;
    CurrentMoveKey = InMoveKey;
    CurrentMoveId = InMoveId;
}

void UAttackStateComponent::StartActivePhase()
{
    if (Phase == EAttackPhase::Anticipation)
    {
        InternalSetPhase(EAttackPhase::Active);
        PhaseTime = 0.0f;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[AttackStateComponent]: Cannot start Active phase from %s"),
               *UEnum::GetValueAsString(Phase));
    }
}

void UAttackStateComponent::StartRecoveryPhase()
{
    if (Phase == EAttackPhase::Active)
    {
        InternalSetPhase(EAttackPhase::Recovery);
        PhaseTime = 0.0f;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[AttackStateComponent]: Cannot start Recovery phase from %s"),
               *UEnum::GetValueAsString(Phase));
    }
}

void UAttackStateComponent::ClearAttackState()
{
    ResetAttackState();
}

// Internal functions
void UAttackStateComponent::InternalSetPhase(EAttackPhase NewPhase)
{
    if (Phase == NewPhase)
    {
        return;
    }

    EAttackPhase OldPhase = Phase;
    Phase = NewPhase;
    
    // Reset phase time when changing phases
    PhaseTime = 0.0f;
    
    // Broadcast phase change
    OnAttackPhaseChanged.Broadcast(NewPhase, OldPhase);
    
    UE_LOG(LogTemp, Log, TEXT("[AttackStateComponent]: Phase changed from %s to %s"),
           *UEnum::GetValueAsString(OldPhase),
           *UEnum::GetValueAsString(NewPhase));
}

void UAttackStateComponent::InternalSetAttackIndex(int32 NewIndex)
{
    if (AttackIndex == NewIndex)
    {
        return;
    }

    int32 OldIndex = AttackIndex;
    AttackIndex = NewIndex;
    
    // Broadcast index change
    OnAttackIndexChanged.Broadcast(NewIndex, OldIndex);
    
    UE_LOG(LogTemp, Log, TEXT("[AttackStateComponent]: Attack index changed from %d to %d"),
           OldIndex, NewIndex);
}
