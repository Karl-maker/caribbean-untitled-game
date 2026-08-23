#include "CombatDetectionSystem.h"

#include "ActorRegistry.h"

#include "../Components/CombatVolumeComponent.h"
#include "../EventBus/EventBusSubsystem.h"

#include "Components/ShapeComponent.h"
#include "Components/PrimitiveComponent.h"

#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"


// ============================================================================
// Combat Collision Channel
// ============================================================================
//
// Create this in:
//
// Project Settings
//     -> Engine
//         -> Collision
//             -> Object Channels
//
// Add:
//
//     CombatVolume
//
// The first custom object channel is normally:
//
//     ECC_GameTraceChannel1
//
// If your project assigned CombatVolume to a different custom channel,
// change this definition accordingly.
// ============================================================================

#define ECC_CombatVolume ECC_GameTraceChannel1


// ============================================================================
// Tick
// ============================================================================

void UCombatDetectionSystem::Tick(UWorld* World, float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    if (!World)
    {
        return;
    }

    // ========================================================================
    // Server Authority
    // ========================================================================
    //
    // Combat detection should be authoritative.
    //
    // Clients do not decide whether a combat collision happened.
    //
    // The server performs the overlap queries and publishes the resulting
    // combat collision event through the server's EventBus.
    // ========================================================================

    if (World->GetNetMode() == NM_Client)
    {
        return;
    }

    // ========================================================================
    // Detection Frequency
    // ========================================================================
    //
    // The scheduler can tick this system frequently.
    //
    // We intentionally only perform collision detection every
    // DetectionTickInterval ticks.
    //
    // Example:
    //
    // Scheduler:
    //
    //     Tick 1 -> skip
    //     Tick 2 -> detect
    //     Tick 3 -> skip
    //     Tick 4 -> detect
    //
    // This reduces the number of expensive spatial queries.
    // ========================================================================

    ++DetectionTickCounter;

    if (DetectionTickCounter < DetectionTickInterval)
    {
        return;
    }

    DetectionTickCounter = 0;

    DetectCollisions(World);
}


// ============================================================================
// Detection
// ============================================================================

void UCombatDetectionSystem::DetectCollisions(UWorld* World)
{
    if (!World)
    {
        return;
    }

    UActorRegistry* Registry =
        World->GetSubsystem<UActorRegistry>();

    if (!Registry)
    {
        return;
    }

    // ========================================================================
    // Actor Registry
    // ========================================================================
    //
    // We do NOT search every actor in the world.
    //
    // ActorRegistry already maintains actors containing
    // UCombatVolumeComponent.
    // ========================================================================

    const TSet<TWeakObjectPtr<AActor>>* Actors =
        Registry->GetActorsWithComponent<UCombatVolumeComponent>();

    if (!Actors)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *Actors)
    {
        AActor* Actor = WeakActor.Get();

        if (!IsValid(Actor))
        {
            continue;
        }

        ProcessActor(Actor);
    }
}


// ============================================================================
// Actor
// ============================================================================

void UCombatDetectionSystem::ProcessActor(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }

    // ========================================================================
    // Multiple Combat Volumes
    // ========================================================================
    //
    // An actor can have many combat volumes.
    //
    // Example:
    //
    // Character
    //     Head
    //     Torso
    //     LeftArm
    //     RightArm
    //     LeftLeg
    //     RightLeg
    //
    // Or:
    //
    // Character
    //     Body volumes
    //
    // Weapon
    //     Blade
    //     Handle
    //
    // Therefore FindComponentByClass() cannot be used here.
    // ========================================================================

    TArray<UCombatVolumeComponent*> CombatVolumes;

    Actor->GetComponents<UCombatVolumeComponent>(
        CombatVolumes
    );

    for (UCombatVolumeComponent* CombatVolume : CombatVolumes)
    {
        if (!IsValid(CombatVolume))
        {
            continue;
        }

        // ====================================================================
        // Cheap Check #1
        // ====================================================================
        //
        // Disabled combat volumes do absolutely no collision work.
        // ====================================================================

        if (!CombatVolume->IsEnabled())
        {
            continue;
        }

        // ====================================================================
        // Cheap Check #2
        // ====================================================================
        //
        // Only attack-active volumes are collision SOURCES.
        //
        // This does NOT mean the target volume must be attacking.
        //
        // Example:
        //
        // Sword:
        //     AttackActive = true
        //
        // Enemy Torso:
        //     AttackActive = false
        //
        // Sword -> Torso is still detected.
        //
        // The target's AttackActive state is intentionally irrelevant here.
        // ====================================================================

        if (!CombatVolume->IsAttackActive())
        {
            continue;
        }

        ProcessCombatVolume(CombatVolume);
    }
}


// ============================================================================
// Combat Volume
// ============================================================================

void UCombatDetectionSystem::ProcessCombatVolume(
    UCombatVolumeComponent* CombatVolume
)
{
    if (!IsValid(CombatVolume))
    {
        return;
    }

    // ========================================================================
    // Get Shape
    // ========================================================================

    UShapeComponent* Shape =
        CombatVolume->GetCollisionShape();

    if (!IsValid(Shape))
    {
        return;
    }

    // ========================================================================
    // Collision State
    // ========================================================================

    if (Shape->GetCollisionEnabled() ==
        ECollisionEnabled::NoCollision)
    {
        return;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    // ========================================================================
    // Query Parameters
    // ========================================================================

    TArray<FOverlapResult> Overlaps;

    FCollisionQueryParams QueryParams(
        TEXT("CombatVolumeDetection"),
        false
    );

    // ========================================================================
    // Ignore Source Actor
    // ========================================================================
    //
    // Prevent:
    //
    //     Sword -> owner's arm
    //     Sword -> owner's torso
    //     Fist  -> owner's body
    //
    // from producing combat collisions.
    // ========================================================================

    AActor* SourceActor =
        CombatVolume->GetOwner();

    if (IsValid(SourceActor))
    {
        QueryParams.AddIgnoredActor(SourceActor);
    }

    // ========================================================================
    // Build Query Shape
    // ========================================================================
    //
    // UE performs the actual spatial collision test.
    //
    // We don't manually implement:
    //
    //     Box vs Box
    //     Sphere vs Sphere
    //     Capsule vs Box
    //     Capsule vs Sphere
    //     etc.
    //
    // UE's collision system handles this.
    // ========================================================================

    FCollisionShape QueryShape;

    switch (CombatVolume->GetShapeType())
    {
        case ECombatVolumeShape::Box:
        {
            const FVector Extent =
                CombatVolume->GetBoxExtent();

            QueryShape =
                FCollisionShape::MakeBox(Extent);

            break;
        }

        case ECombatVolumeShape::Sphere:
        {
            const float Radius =
                CombatVolume->GetSphereRadius();

            if (Radius <= 0.0f)
            {
                return;
            }

            QueryShape =
                FCollisionShape::MakeSphere(Radius);

            break;
        }

        case ECombatVolumeShape::Capsule:
        {
            const float Radius =
                CombatVolume->GetCapsuleRadius();

            const float HalfHeight =
                CombatVolume->GetCapsuleHalfHeight();

            if (Radius <= 0.0f ||
                HalfHeight <= 0.0f)
            {
                return;
            }

            QueryShape =
                FCollisionShape::MakeCapsule(
                    Radius,
                    HalfHeight
                );

            break;
        }

        default:
        {
            return;
        }
    }

    // ========================================================================
    // Transform
    // ========================================================================

    const FVector Location =
        Shape->GetComponentLocation();

    const FQuat Rotation =
        Shape->GetComponentQuat();

    // ========================================================================
    // UE Spatial Query
    // ========================================================================
    //
    // IMPORTANT:
    //
    // We only query the dedicated CombatVolume collision channel.
    //
    // Therefore this query doesn't care about normal gameplay collision
    // channels.
    //
    // It is specifically asking:
    //
    // "Which combat volumes are inside this combat volume?"
    // ========================================================================

    const bool bHasOverlap =
        World->OverlapMultiByChannel(
            Overlaps,
            Location,
            Rotation,
            ECC_CombatVolume,
            QueryShape,
            QueryParams
        );

    if (!bHasOverlap)
    {
        return;
    }

    // ========================================================================
    // Process Results
    // ========================================================================

    for (const FOverlapResult& Overlap : Overlaps)
    {
        ProcessOverlap(
            CombatVolume,
            Overlap
        );
    }
}


// ============================================================================
// Overlap
// ============================================================================

void UCombatDetectionSystem::ProcessOverlap(
    UCombatVolumeComponent* SourceVolume,
    const FOverlapResult& Overlap
)
{
    if (!IsValid(SourceVolume))
    {
        return;
    }

    // ========================================================================
    // Get Target Primitive
    // ========================================================================
    //
    // FOverlapResult contains the component that was overlapped.
    //
    // Engine/OverlapResult.h is explicitly included above so FOverlapResult
    // is a complete type here.
    // ========================================================================

    UPrimitiveComponent* TargetComponent =
        Overlap.Component.Get();

    if (!IsValid(TargetComponent))
    {
        return;
    }

    // ========================================================================
    // Target Actor
    // ========================================================================

    AActor* SourceActor =
        SourceVolume->GetOwner();

    AActor* TargetActor =
        TargetComponent->GetOwner();

    if (!IsValid(TargetActor))
    {
        return;
    }

    // ========================================================================
    // Ignore Self
    // ========================================================================
    //
    // This is normally already handled by QueryParams, but keeping this
    // check makes the function safe if it is ever called from another path.
    // ========================================================================

    if (TargetActor == SourceActor)
    {
        return;
    }

    // ========================================================================
    // Find Target Combat Volume
    // ========================================================================
    //
    // Since the query is against ECC_CombatVolume, the target should be one
    // of our generated combat shapes.
    //
    // We verify which UCombatVolumeComponent owns the shape.
    // ========================================================================

    UCombatVolumeComponent* TargetVolume = nullptr;

    TArray<UCombatVolumeComponent*> TargetVolumes;

    TargetActor->GetComponents<UCombatVolumeComponent>(
        TargetVolumes
    );

    for (UCombatVolumeComponent* Candidate : TargetVolumes)
    {
        if (!IsValid(Candidate))
        {
            continue;
        }

        if (Candidate->GetCollisionShape() == TargetComponent)
        {
            TargetVolume = Candidate;
            break;
        }
    }

    // ========================================================================
    // Only Combat Volumes
    // ========================================================================
    //
    // If the component wasn't actually owned by a CombatVolumeComponent,
    // ignore the result.
    // ========================================================================

    if (!TargetVolume)
    {
        return;
    }

    // ========================================================================
    // Target Enabled Check
    // ========================================================================
    //
    // The target may have been disabled after the query configuration was
    // established.
    //
    // Don't publish a collision against a disabled target volume.
    // ========================================================================

    if (!TargetVolume->IsEnabled())
    {
        return;
    }

    // ========================================================================
    // Build Payload
    // ========================================================================

    FCombatCollisionPayload Payload;

    // ------------------------------------------------------------------------
    // Source
    // ------------------------------------------------------------------------

    Payload.SourceVolume =
        SourceVolume;

    Payload.SourceActor =
        IsValid(SourceActor)
            ? SourceActor
            : nullptr;

    // ------------------------------------------------------------------------
    // Target
    // ------------------------------------------------------------------------

    Payload.TargetVolume =
        TargetVolume;

    Payload.TargetActor =
        IsValid(TargetActor)
            ? TargetActor
            : nullptr;

    Payload.TargetComponent =
        TargetComponent;

    // ========================================================================
    // Source Attachment
    // ========================================================================
    //
    // This tells the receiver what the attacking volume is attached to.
    //
    // Examples:
    //
    //     Character Mesh
    //     RightHand socket
    //
    // or:
    //
    //     Sword Mesh
    //     Blade socket
    //
    // or:
    //
    //     Shield Mesh
    //     Shield socket
    // ========================================================================

    UShapeComponent* SourceShape =
        SourceVolume->GetCollisionShape();

    if (IsValid(SourceShape))
    {
        Payload.SourceAttachedTo =
            SourceShape->GetAttachParent();

        Payload.SourceAttachSocket =
            SourceShape->GetAttachSocketName();

        Payload.Location =
            SourceShape->GetComponentLocation();
    }

    // ========================================================================
    // Publish
    // ========================================================================

    PublishCollisionEvent(Payload);
}


// ============================================================================
// Event Bus
// ============================================================================

void UCombatDetectionSystem::PublishCollisionEvent(
    const FCombatCollisionPayload& Payload
)
{
    UEventBusSubsystem* EventBus =
        GetEventBus();

    if (!EventBus)
    {
        return;
    }

    EventBus->Publish(Payload);
}


UEventBusSubsystem* UCombatDetectionSystem::GetEventBus() const
{
    const UWorld* World = GetWorld();

    if (!World)
    {
        return nullptr;
    }

    return World->GetSubsystem<UEventBusSubsystem>();
}


// ============================================================================
// Cleanup
// ============================================================================

void UCombatDetectionSystem::Cleanup()
{
    DetectionTickCounter = 0;
}
