#include "CombatVolumeComponent.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ShapeComponent.h"


UCombatVolumeComponent::UCombatVolumeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    /*
     * The combat volume is a query object, not a physics body.
     *
     * The actual collision shape will be created when the
     * component is registered with its owning Actor.
     */
}


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

    CollisionShape->CreationMethod = EComponentCreationMethod::Instance;

    Owner->AddInstanceComponent(CollisionShape);

    CollisionShape->RegisterComponent();

    ConfigureCollisionShape();

    CollisionShape->SetCollisionEnabled(
        bStartEnabled
            ? ECollisionEnabled::QueryOnly
            : ECollisionEnabled::NoCollision
    );
}


void UCombatVolumeComponent::ConfigureCollisionShape()
{
    if (!CollisionShape)
    {
        return;
    }

    /*
     * Combat volumes are query-only.
     *
     * They should not participate in physics simulation.
     */
    CollisionShape->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly
    );

    CollisionShape->SetGenerateOverlapEvents(true);

    /*
     * We will eventually give combat volumes their own
     * collision object type/channel.
     *
     * For now, QueryOnly keeps the volume out of physics.
     */

    switch (ShapeType)
    {
        case ECombatVolumeShape::Box:
        {
            UBoxComponent* BoxComponent =
                Cast<UBoxComponent>(CollisionShape);

            if (BoxComponent)
            {
                BoxComponent->SetBoxExtent(BoxExtent);
            }

            break;
        }

        case ECombatVolumeShape::Sphere:
        {
            USphereComponent* SphereComponent =
                Cast<USphereComponent>(CollisionShape);

            if (SphereComponent)
            {
                SphereComponent->SetSphereRadius(SphereRadius);
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

    const bool bAttached =
        CollisionShape->AttachToComponent(
            Parent,
            AttachRules,
            SocketName
        );

    return bAttached;
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
