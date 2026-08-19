#include "CombatVolumeComponent.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ShapeComponent.h"

// ============================================================================
// Combat Collision Channel
// ============================================================================
//
// Create this object channel in:
// Project Settings -> Collision -> Object Channels
//
// Make sure GameTraceChannel1 is named "CombatVolume".
// ============================================================================

#define ECC_CombatVolume ECC_GameTraceChannel1


UCombatVolumeComponent::UCombatVolumeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    /*
     * The combat volume itself does not tick.
     *
     * CombatDetectionSystem is responsible for performing
     * collision queries at the desired frequency.
     *
     * This keeps individual combat volumes extremely cheap.
     */
}


// ============================================================================
// Registration
// ============================================================================

void UCombatVolumeComponent::OnRegister()
{
    Super::OnRegister();

    CreateCollisionShape();
}


void UCombatVolumeComponent::OnUnregister()
{
    if (CollisionShape)
    {
        CollisionShape->DestroyComponent();
        CollisionShape = nullptr;
    }

    Super::OnUnregister();
}


// ============================================================================
// Shape Creation
// ============================================================================

void UCombatVolumeComponent::CreateCollisionShape()
{
    if (CollisionShape)
    {
        return;
    }

    AActor* Owner = GetOwner();

    if (!Owner)
    {
        return;
    }

    switch (ShapeType)
    {
        case ECombatVolumeShape::Box:
        {
            UBoxComponent* BoxComponent =
                NewObject<UBoxComponent>(
                    Owner,
                    NAME_None,
                    RF_Transient
                );

            CollisionShape = BoxComponent;
            break;
        }

        case ECombatVolumeShape::Sphere:
        {
            USphereComponent* SphereComponent =
                NewObject<USphereComponent>(
                    Owner,
                    NAME_None,
                    RF_Transient
                );

            CollisionShape = SphereComponent;
            break;
        }

        case ECombatVolumeShape::Capsule:
        {
            UCapsuleComponent* CapsuleComponent =
                NewObject<UCapsuleComponent>(
                    Owner,
                    NAME_None,
                    RF_Transient
                );

            CollisionShape = CapsuleComponent;
            break;
        }

        default:
        {
            break;
        }
    }

    if (!CollisionShape)
    {
        return;
    }

    CollisionShape->CreationMethod =
        EComponentCreationMethod::Instance;

    Owner->AddInstanceComponent(CollisionShape);

    ConfigureCollisionShape();

    CollisionShape->RegisterComponent();

    /*
     * ConfigureCollisionShape() sets the collision mode based
     * on bStartEnabled.
     */
    CollisionShape->SetCollisionEnabled(
        bStartEnabled
            ? ECollisionEnabled::QueryOnly
            : ECollisionEnabled::NoCollision
    );
}


// ============================================================================
// Collision Configuration
// ============================================================================

void UCombatVolumeComponent::ConfigureCollisionShape()
{
    if (!CollisionShape)
    {
        return;
    }

    /*
     * Combat volumes are query-only.
     *
     * They should never participate in physics simulation.
     */
    CollisionShape->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly
    );

    /*
     * Put every combat volume onto the dedicated
     * CombatVolume object channel.
     */
    CollisionShape->SetCollisionObjectType(
        ECC_CombatVolume
    );

    /*
     * CombatDetectionSystem performs explicit collision
     * queries using OverlapMultiByChannel().
     *
     * Therefore we do NOT need UE's automatic overlap events.
     *
     * This avoids unnecessary overlap event processing.
     */
    CollisionShape->SetGenerateOverlapEvents(false);

    /*
     * Ignore everything by default.
     *
     * Combat detection only cares about other combat volumes.
     */
    CollisionShape->SetCollisionResponseToAllChannels(
        ECR_Ignore
    );

    /*
     * Combat volumes overlap other combat volumes.
     */
    CollisionShape->SetCollisionResponseToChannel(
        ECC_CombatVolume,
        ECR_Overlap
    );


    // ------------------------------------------------------------------------
    // Configure Shape Size
    // ------------------------------------------------------------------------

    switch (ShapeType)
    {
        case ECombatVolumeShape::Box:
        {
            UBoxComponent* BoxComponent =
                Cast<UBoxComponent>(CollisionShape);

            if (BoxComponent)
            {
                BoxComponent->SetBoxExtent(
                    BoxExtent
                );
            }

            break;
        }

        case ECombatVolumeShape::Sphere:
        {
            USphereComponent* SphereComponent =
                Cast<USphereComponent>(CollisionShape);

            if (SphereComponent)
            {
                SphereComponent->SetSphereRadius(
                    SphereRadius
                );
            }

            break;
        }

        case ECombatVolumeShape::Capsule:
        {
            UCapsuleComponent* CapsuleComponent =
                Cast<UCapsuleComponent>(CollisionShape);

            if (CapsuleComponent)
            {
                CapsuleComponent->SetCapsuleSize(
                    CapsuleRadius,
                    CapsuleHalfHeight
                );
            }

            break;
        }

        default:
        {
            break;
        }
    }
}


// ============================================================================
// Attachment
// ============================================================================

bool UCombatVolumeComponent::AttachToComponent(
    USceneComponent* Parent,
    FName SocketName
)
{
    if (!Parent || !CollisionShape)
    {
        return false;
    }

    const FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        false
    );

    return CollisionShape->AttachToComponent(
        Parent,
        AttachRules,
        SocketName
    );
}


void UCombatVolumeComponent::Detach()
{
    if (!CollisionShape)
    {
        return;
    }

    CollisionShape->DetachFromComponent(
        FDetachmentTransformRules::KeepWorldTransform
    );
}


// ============================================================================
// Enable / Disable
// ============================================================================

void UCombatVolumeComponent::SetEnabled(bool bEnabled)
{
    if (!CollisionShape)
    {
        return;
    }

    CollisionShape->SetCollisionEnabled(
        bEnabled
            ? ECollisionEnabled::QueryOnly
            : ECollisionEnabled::NoCollision
    );
}


bool UCombatVolumeComponent::IsEnabled() const
{
    if (!CollisionShape)
    {
        return false;
    }

    return CollisionShape->GetCollisionEnabled()
        != ECollisionEnabled::NoCollision;
}


// ============================================================================
// Attack State
// ============================================================================

void UCombatVolumeComponent::SetAttackActive(bool bActive)
{
    bAttackActive = bActive;
}


bool UCombatVolumeComponent::IsAttackActive() const
{
    return bAttackActive;
}
