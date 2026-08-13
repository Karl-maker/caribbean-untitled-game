//
// PhysicsComponent.cpp
// CaribbeanUntitled
//

#include "RigidbodyComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"


URigidbodyComponent::URigidbodyComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


void URigidbodyComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!GetOwner())
    {
        return;
    }

    /*
     * Find the primitive component that represents
     * this actor's physical body.
     *
     * This can be your mesh, capsule, box, etc.
     */
    PhysicsBody =
        GetOwner()->FindComponentByClass<UPrimitiveComponent>();

    if (!PhysicsBody)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PhysicsComponent on %s has no UPrimitiveComponent."),
            *GetOwner()->GetName()
        );

        return;
    }

    PhysicsBody->SetCollisionEnabled(
        CollisionEnabled
    );

    PhysicsBody->SetCollisionObjectType(
        CollisionChannel
    );

    PhysicsBody->SetSimulatePhysics(
        bSimulatePhysics
    );

    PhysicsBody->SetEnableGravity(
        bEnableGravity
    );

    if (Mass > 0.0f)
    {
        PhysicsBody->SetMassOverrideInKg(
            NAME_None,
            Mass,
            true
        );
    }
}


void URigidbodyComponent::AddForce(
    const FVector& Force)
{
    PendingForce += Force;
}


void URigidbodyComponent::AddImpulse(
    const FVector& Impulse)
{
    PendingImpulse += Impulse;
}


void URigidbodyComponent::AddTorque(
    const FVector& Torque)
{
    PendingTorque += Torque;
}


void URigidbodyComponent::AddAngularImpulse(
    const FVector& Impulse)
{
    PendingAngularImpulse += Impulse;
}


bool URigidbodyComponent::Sweep(
    const FVector& Start,
    const FVector& End,
    FHitResult& Hit) const
{
    if (!PhysicsBody)
    {
        return false;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return false;
    }

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    return World->SweepSingleByChannel(
        Hit,
        Start,
        End,
        FQuat::Identity,
        CollisionChannel,
        PhysicsBody->GetCollisionShape(),
        Params
    );
}


bool URigidbodyComponent::IsSimulatingPhysics() const
{
    return PhysicsBody &&
           PhysicsBody->IsSimulatingPhysics();
}


UPrimitiveComponent* URigidbodyComponent::GetPhysicsBody() const
{
    return PhysicsBody;
}


void URigidbodyComponent::ConsumePendingForces()
{
    if (!PhysicsBody)
    {
        return;
    }

    if (!PendingForce.IsNearlyZero())
    {
        PhysicsBody->AddForce(
            PendingForce
        );
    }

    if (!PendingImpulse.IsNearlyZero())
    {
        PhysicsBody->AddImpulse(
            PendingImpulse
        );
    }

    if (!PendingTorque.IsNearlyZero())
    {
        PhysicsBody->AddTorqueInRadians(
            PendingTorque
        );
    }

    if (!PendingAngularImpulse.IsNearlyZero())
    {
        PhysicsBody->AddAngularImpulseInRadians(
            PendingAngularImpulse
        );
    }

    PendingForce = FVector::ZeroVector;
    PendingImpulse = FVector::ZeroVector;
    PendingTorque = FVector::ZeroVector;
    PendingAngularImpulse = FVector::ZeroVector;
}
