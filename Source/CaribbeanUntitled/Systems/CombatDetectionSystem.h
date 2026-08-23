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


    // ---------------------------------------------------------
    // Event Bus
    // ---------------------------------------------------------

    void PublishCollisionEvent(
        const FCombatCollisionPayload& Payload
    );

    UEventBusSubsystem* GetEventBus() const;


private:

    /**
     * Run combat detection every other scheduler tick.
     *
     * At 60 FPS:
     *
     * Game:
     *  1 2 3 4 5 6
     *
     * Combat:
     *   X   X   X
     *
     * ~30 detection passes per second.
     */
    static constexpr int32 DetectionTickInterval = 2;

    int32 DetectionTickCounter = 0;
};
