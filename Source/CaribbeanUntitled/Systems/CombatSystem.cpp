//
// CombatSystem.cpp
// CaribbeanUntitled
//

#include "CombatSystem.h"

#include "../EventBus/EventBusSubsystem.h"
#include "../Components/AttackStateComponent.h"
#include "../Components/CombatComponent.h"
#include "../Components/CombatVolumeComponent.h"
#include "../Components/ControlComponent.h"
#include "../Components/MoveSetComponent.h"
#include "../Components/StatsComponent.h"
#include "CombatDetectionSystem.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"


//=============================================================================
// Lifecycle
//=============================================================================

void UCombatSystem::Initialize(
    FSubsystemCollectionBase& Collection
)
{
    Super::Initialize(Collection);


    SubscribeToEvents();
}


void UCombatSystem::Deinitialize()
{
    UnsubscribeFromEvents();


    Super::Deinitialize();
}


//=============================================================================
// Event Bus
//=============================================================================

UEventBusSubsystem* UCombatSystem::GetEventBus() const
{
    const UWorld* World = GetWorld();

    if (!World)
    {
        return nullptr;
    }

    return World->GetSubsystem<UEventBusSubsystem>();
}


void UCombatSystem::SubscribeToEvents()
{
    UEventBusSubsystem* EventBus = GetEventBus();

    if (!EventBus)
    {

        return;
    }

    EventBus->Subscribe<FCombatCollisionPayload>(
        this,
        &UCombatSystem::OnCombatCollision
    );

}


void UCombatSystem::UnsubscribeFromEvents()
{
    UEventBusSubsystem* EventBus = GetEventBus();

    if (!EventBus)
    {
        return;
    }

    EventBus->Unsubscribe(this);
}


//=============================================================================
// Combat Events
//=============================================================================

void UCombatSystem::OnCombatCollision(
    const FCombatCollisionPayload& Payload
)
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client)
    {
        return;
    }

    AActor* SourceActor = Payload.SourceActor;
    AActor* TargetActor = Payload.TargetActor;
    if (!IsValid(SourceActor) ||
        !IsValid(TargetActor) ||
        SourceActor == TargetActor)
    {
        return;
    }

    const UAttackStateComponent* AttackState =
        SourceActor->FindComponentByClass<UAttackStateComponent>();
    const UMoveSetComponent* MoveSet =
        SourceActor->FindComponentByClass<UMoveSetComponent>();
    UStatsComponent* TargetStats =
        TargetActor->FindComponentByClass<UStatsComponent>();

    if (!AttackState || !MoveSet || !TargetStats || TargetStats->IsDead())
    {
        return;
    }

    const FGameplayCombatMove Move =
        MoveSet->GetMove(AttackState->GetCurrentMoveKey());
    if (Move.MoveId.IsNone())
    {
        return;
    }

    const UStatsComponent* SourceStats =
        SourceActor->FindComponentByClass<UStatsComponent>();

    const float RawDamage = FMath::Max(
        0.0f,
        Move.Damage + (SourceStats ? SourceStats->GetStrength() : 0.0f)
    );
    float DamageToApply = RawDamage;

    bool bBlocked = false;
    if (IsBlockEffective(SourceActor, TargetActor))
    {
        const float BlockCost = TargetStats->GetBlockStaminaCost();
        if (TargetStats->ConsumeStamina(BlockCost))
        {
            DamageToApply *=
                1.0f - TargetStats->GetBlockDamageReduction();
            bBlocked = true;
        }
    }

    const float HealthBefore = TargetStats->GetHealth();
    if (!TargetStats->ApplyDamage(DamageToApply))
    {
        return;
    }

    FCombatResolvedPayload Resolved;
    Resolved.SourceActor = SourceActor;
    Resolved.TargetActor = TargetActor;
    Resolved.RawDamage = RawDamage;
    Resolved.AppliedDamage =
        FMath::Max(0.0f, HealthBefore - TargetStats->GetHealth());
    Resolved.bBlocked = bBlocked;
    Resolved.bKilled = TargetStats->IsDead();
    Resolved.HitLocation = Payload.Location;

    if (UEventBusSubsystem* EventBus = GetEventBus())
    {
        EventBus->Publish(Resolved);
    }
}

bool UCombatSystem::IsBlockEffective(
    AActor* SourceActor,
    AActor* TargetActor) const
{
    if (!IsValid(SourceActor) || !IsValid(TargetActor))
    {
        return false;
    }

    const UControlComponent* Control =
        TargetActor->FindComponentByClass<UControlComponent>();
    const UStatsComponent* Stats =
        TargetActor->FindComponentByClass<UStatsComponent>();
    if (!Control || !Stats || !Control->WantsToBlock())
    {
        return false;
    }

    FVector ToSource =
        SourceActor->GetActorLocation() - TargetActor->GetActorLocation();
    ToSource.Z = 0.0f;
    if (!ToSource.Normalize())
    {
        return true;
    }

    FVector Forward = TargetActor->GetActorForwardVector();
    Forward.Z = 0.0f;
    Forward.Normalize();

    const float HalfAngleRadians =
        FMath::DegreesToRadians(Stats->GetBlockAngle() * 0.5f);

    return FVector::DotProduct(Forward, ToSource) >=
        FMath::Cos(HalfAngleRadians);
}
