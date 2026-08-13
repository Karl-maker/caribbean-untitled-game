//
// RigidbodyComponent.cpp
// CaribbeanUntitled
//

#include "RigidbodyComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"


URigidbodyComponent::URigidbodyComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


// ============================================================================
// LIFECYCLE
// ============================================================================

void URigidbodyComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    PhysicsBody = Owner->FindComponentByClass<UPrimitiveComponent>();

    if (!PhysicsBody)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("RigidbodyComponent on %s has no UPrimitiveComponent."),
            *Owner->GetName()
        );

        return;
    }

    // Collision
    PhysicsBody->SetCollisionEnabled(
        bEnableCollision
            ? CollisionEnabled.GetValue()
            : ECollisionEnabled::NoCollision
    );

    PhysicsBody->SetCollisionObjectType(CollisionChannel);

    // Chaos simulation
    PhysicsBody->SetSimulatePhysics(bSimulatePhysics);

    PhysicsBody->SetEnableGravity(bEnableGravity);

    // Mass
    if (Mass > 0.0f)
    {
        PhysicsBody->SetMassOverrideInKg(
            NAME_None,
            Mass,
            true
        );
    }
}


// ============================================================================
// INTERNAL SETUP
// ============================================================================

void URigidbodyComponent::ResolvePhysicsBody()
{
    if (!GetOwner())
    {
        return;
    }

    /*
     * The RigidbodyComponent does not own the collision geometry.
     *
     * The actual physics body belongs to a UPrimitiveComponent such as:
     *
     *   UStaticMeshComponent
     *   USkeletalMeshComponent
     *   UCapsuleComponent
     *   UBoxComponent
     *   USphereComponent
     *
     * Chaos operates on that component.
     */
    PhysicsBody =
        GetOwner()->FindComponentByClass<UPrimitiveComponent>();
}


void URigidbodyComponent::ConfigurePhysicsBody()
{
    if (!PhysicsBody)
    {
        return;
    }

    // ------------------------------------------------------------
    // COLLISION
    // ------------------------------------------------------------

    if (bEnableCollision)
    {
        PhysicsBody->SetCollisionEnabled(CollisionEnabled.GetValue());
    }
    else
    {
        PhysicsBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    PhysicsBody->SetCollisionObjectType(
        CollisionChannel
    );

    // ------------------------------------------------------------
    // CHAOS SIMULATION
    // ------------------------------------------------------------

    PhysicsBody->SetSimulatePhysics(
        bSimulatePhysics
    );

    PhysicsBody->SetEnableGravity(
        bEnableGravity
    );

    // ------------------------------------------------------------
    // MASS
    // ------------------------------------------------------------

    if (Mass > 0.0f)
    {
        PhysicsBody->SetMassOverrideInKg(
            NAME_None,
            Mass,
            true
        );
    }
}


// ============================================================================
// FORCE / IMPULSE REQUESTS
// ============================================================================

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


// ============================================================================
// PHYSICS BODY
// ============================================================================

UPrimitiveComponent* URigidbodyComponent::GetPhysicsBody() const
{
    return PhysicsBody;
}


bool URigidbodyComponent::HasPhysicsBody() const
{
    return PhysicsBody != nullptr;
}


bool URigidbodyComponent::IsSimulatingPhysics() const
{
    return PhysicsBody &&
           PhysicsBody->IsSimulatingPhysics();
}


// ============================================================================
// PHYSICS STATE
// ============================================================================

void URigidbodyComponent::UpdatePhysicsState()
{
    if (!PhysicsBody)
    {
        return;
    }

    /*
     * These values are snapshots of Chaos state.
     *
     * They are NOT independently simulated by RigidbodyComponent.
     */

    LinearVelocity =
        PhysicsBody->GetPhysicsLinearVelocity();

    AngularVelocity =
        PhysicsBody->GetPhysicsAngularVelocityInRadians();

    PhysicsLocation =
        PhysicsBody->GetComponentLocation();

    PhysicsRotation =
        PhysicsBody->GetComponentRotation();
}


void URigidbodyComponent::SetGrounded(
    bool bGrounded,
    const FVector& Normal)
{
    bIsGrounded = bGrounded;

    if (bGrounded)
    {
        GroundNormal =
            Normal.GetSafeNormal();
    }
    else
    {
        GroundNormal =
            FVector::UpVector;
    }
}


void URigidbodyComponent::ClearGroundedState()
{
    bIsGrounded = false;
    GroundNormal = FVector::UpVector;
}


// ============================================================================
// COLLISION QUERIES
// ============================================================================

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

    FCollisionQueryParams Params(
        SCENE_QUERY_STAT(RigidbodySweep),
        false
    );

    if (GetOwner())
    {
        Params.AddIgnoredActor(GetOwner());
    }

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


// ============================================================================
// FORCE CONSUMPTION
// ============================================================================

void URigidbodyComponent::ConsumePendingForces()
{
    if (!PhysicsBody)
    {
        return;
    }

    /*
     * Forces are only meaningful for simulated Chaos bodies.
     *
     * Requests are still cleared if the body isn't simulating so stale
     * requests don't survive into a later frame.
     */

    if (PhysicsBody->IsSimulatingPhysics())
    {
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
    }

    // Requests are frame-local.
    PendingForce = FVector::ZeroVector;
    PendingImpulse = FVector::ZeroVector;
    PendingTorque = FVector::ZeroVector;
    PendingAngularImpulse = FVector::ZeroVector;
}


// ============================================================================
// SIMULATION CONTROL
// ============================================================================

void URigidbodyComponent::SetSimulatePhysics(
    bool bSimulate)
{
    bSimulatePhysics = bSimulate;

    if (!PhysicsBody)
    {
        return;
    }

    PhysicsBody->SetSimulatePhysics(
        bSimulate
    );

    if (!bSimulate)
    {
        UpdatePhysicsState();
    }
}


void URigidbodyComponent::SetEnableGravity(
    bool bEnable)
{
    bEnableGravity = bEnable;

    if (!PhysicsBody)
    {
        return;
    }

    PhysicsBody->SetEnableGravity(
        bEnable
    );
}


// ============================================================================
// VELOCITY CONTROL
// ============================================================================

void URigidbodyComponent::SetLinearVelocity(
    const FVector& Velocity)
{
    if (!PhysicsBody)
    {
        return;
    }

    PhysicsBody->SetPhysicsLinearVelocity(
        Velocity
    );

    LinearVelocity = Velocity;
}


void URigidbodyComponent::SetAngularVelocity(
    const FVector& Velocity)
{
    if (!PhysicsBody)
    {
        return;
    }

    /*
     * Chaos expects angular velocity in radians/sec.
     */
    PhysicsBody->SetPhysicsAngularVelocityInRadians(
        Velocity
    );

    AngularVelocity = Velocity;
}
