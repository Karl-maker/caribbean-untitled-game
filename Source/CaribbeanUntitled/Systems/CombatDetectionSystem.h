#pragma once

#include "CoreMinimal.h"
#include "CombatDetectionSystem.generated.h"

class UCombatVolumeComponent;
class UPrimitiveComponent;
class USceneComponent;
class UEventBusSubsystem;

/**
 * Pure collision-detection event.
 *
 * This system does NOT determine damage, blocking,
 * hit reactions, stun, etc.
 */
USTRUCT()
struct FCombatCollisionPayload
{
    GENERATED_BODY()

    // ---------------------------------------------------------
    // Source
    // ---------------------------------------------------------

    /**
     * Combat volume that detected the collision.
     *
     * Example:
     * SwordVolume
     */
    UPROPERTY()
    TObjectPtr<UCombatVolumeComponent> SourceVolume = nullptr;

    /**
     * Actor that owns SourceVolume.
     *
     * nullptr if there is no owning actor.
     */
    UPROPERTY()
    TObjectPtr<AActor> SourceActor = nullptr;

    /**
     * Component that SourceVolume is attached to.
     *
     * Example:
     * Character Mesh
     * Sword Mesh
     *
     * nullptr if not attached.
     */
    UPROPERTY()
    TObjectPtr<USceneComponent> SourceAttachedTo = nullptr;

    /**
     * Socket SourceVolume is attached to.
     */
    UPROPERTY()
    FName SourceAttachSocket = NAME_None;


    // ---------------------------------------------------------
    // Target
    // ---------------------------------------------------------

    /**
     * Combat volume that was detected.
     *
     * nullptr if the collision was with something that
     * is not a combat volume.
     */
    UPROPERTY()
    TObjectPtr<UCombatVolumeComponent> TargetVolume = nullptr;

    /**
     * Actor that owns TargetVolume / OtherComponent.
     *
     * nullptr if there is no owning actor.
     */
    UPROPERTY()
    TObjectPtr<AActor> TargetActor = nullptr;

    /**
     * Actual primitive component that was overlapped.
     */
    UPROPERTY()
    TObjectPtr<UPrimitiveComponent> TargetComponent = nullptr;


    // ---------------------------------------------------------
    // Collision
    // ---------------------------------------------------------

    /**
     * World-space location of the source volume.
     */
    UPROPERTY()
    FVector Location = FVector::ZeroVector;
};


UCLASS()
class CARIBBEANUNTITLED_API UCombatDetectionSystem : public UObject
{
    GENERATED_BODY()

public:

    /**
     * Called by UGameplaySystemScheduler.
     */
    void Tick(UWorld* World, float DeltaTime);

    /**
     * Clears internal state.
     */
    void Cleanup();

private:

    // ---------------------------------------------------------
    // Detection
    // ---------------------------------------------------------

    void DetectCollisions(UWorld* World);

    void ProcessActor(AActor* Actor);

    void ProcessCombatVolume(
        UCombatVolumeComponent* CombatVolume
    );

    void ProcessOverlap(
        UCombatVolumeComponent* SourceVolume,
        const FOverlapResult& Overlap
    );

    bool RegisterHitForCurrentAttack(
        AActor* SourceActor,
        AActor* TargetActor
    );


    // ---------------------------------------------------------
    // Event Bus
    // ---------------------------------------------------------

    void PublishCollisionEvent(
        const FCombatCollisionPayload& Payload
    );

    UEventBusSubsystem* GetEventBus() const;


private:

    /** Run authoritative melee detection at a stable 60 Hz budget. */
    static constexpr float DetectionIntervalSeconds = 1.0f / 60.0f;

    float DetectionAccumulator = 0.0f;

    struct FSourceHitState
    {
        int32 AttackSequence = INDEX_NONE;
        TSet<TWeakObjectPtr<AActor>> HitActors;
    };

    TMap<TWeakObjectPtr<AActor>, FSourceHitState> SourceHitStates;
};
