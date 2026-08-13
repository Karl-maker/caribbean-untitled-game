//
// PhysicsSystem.cpp
// CaribbeanUntitled
//

#include "PhysicsSystem.h"

#include "../Components/RigidbodyComponent.h"
#include "../Components/MotionComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"


// ============================================================================
// SUBSYSTEM
// ============================================================================

void UPhysicsSystem::Initialize(
    FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    Rigidbodies.Reset();
    RigidbodySet.Reset();
}


void UPhysicsSystem::Deinitialize()
{
    Rigidbodies.Reset();
    RigidbodySet.Reset();

    Super::Deinitialize();
}


bool UPhysicsSystem::IsTickable() const
{
    return GetWorld() != nullptr;
}

TStatId UPhysicsSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(
        UPhysicsSystem,
        STATGROUP_Tickables
    );
}

// ============================================================================
// REGISTRATION
// ============================================================================

void UPhysicsSystem::RegisterRigidbody(
    URigidbodyComponent* Rigidbody)
{
    if (!IsValid(Rigidbody))
    {
        return;
    }

    if (RigidbodySet.Contains(Rigidbody))
    {
        return;
    }

    RigidbodySet.Add(Rigidbody);
    Rigidbodies.Add(Rigidbody);
}


void UPhysicsSystem::UnregisterRigidbody(
    URigidbodyComponent* Rigidbody)
{
    if (!Rigidbody)
    {
        return;
    }

    RigidbodySet.Remove(Rigidbody);

    Rigidbodies.RemoveAll(
        [Rigidbody](const TWeakObjectPtr<URigidbodyComponent>& Entry)
        {
            return !Entry.IsValid() ||
                   Entry.Get() == Rigidbody;
        }
    );
}


// ============================================================================
// TICK
// ============================================================================

void UPhysicsSystem::Tick(
    float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    CleanupRegistry();

    SimulatePhysics(DeltaTime);
}


// ============================================================================
// MAIN PHYSICS PIPELINE
// ============================================================================

void UPhysicsSystem::SimulatePhysics(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    CleanupRegistry();

    for (const TWeakObjectPtr<URigidbodyComponent>& Entry : Rigidbodies)
    {
        URigidbodyComponent* Rigidbody = Entry.Get();

        if (!IsValid(Rigidbody))
        {
            continue;
        }

        if (!Rigidbody->HasPhysicsBody())
        {
            continue;
        }

        AActor* Owner = Rigidbody->GetOwner();

        if (!IsValid(Owner))
        {
            continue;
        }

        UMotionComponent* Motion =
            Owner->FindComponentByClass<UMotionComponent>();

        if (Motion)
        {
            ProcessMotion(
                Rigidbody,
                Motion,
                DeltaTime
            );

            ProcessJump(
                Rigidbody,
                Motion
            );
        }

        Rigidbody->ConsumePendingForces();

        UpdatePhysicsState(Rigidbody);

        UpdateGroundedState(Rigidbody);
    }
}

// ============================================================================
// MOTION
// ============================================================================

void UPhysicsSystem::ProcessMotion(
    URigidbodyComponent* Rigidbody,
    UMotionComponent* Motion,
    float DeltaTime)
{
    if (!Rigidbody || !Motion)
    {
        return;
    }

    if (!Rigidbody->IsSimulatingPhysics())
    {
        return;
    }

    if (DeltaTime <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    /*
     * MotionComponent describes what gameplay wants.
     *
     * RigidbodyComponent describes what Chaos is actually doing.
     *
     * PhysicsSystem converts:
     *
     *     DesiredVelocity
     *
     * into:
     *
     *     Force
     */

    const FVector CurrentVelocity =
        Rigidbody->LinearVelocity;

    FVector DesiredVelocity =
        Motion->DesiredVelocity;

    /*
     * Movement requests control horizontal movement.
     *
     * Vertical velocity remains under physics control.
     *
     * Gravity controls falling.
     * Jump controls upward velocity.
     */

    DesiredVelocity.Z = CurrentVelocity.Z;

    FVector VelocityError =
        DesiredVelocity - CurrentVelocity;

    /*
     * We only need to correct horizontal velocity.
     */

    VelocityError.Z = 0.0f;

    if (VelocityError.IsNearlyZero())
    {
        return;
    }

    const float Acceleration =
        Rigidbody->bIsGrounded
            ? MovementAcceleration
            : AirAcceleration;

    /*
     * Required acceleration:
     *
     *     a = Δv / Δt
     */

    FVector DesiredAcceleration =
        VelocityError / DeltaTime;

    DesiredAcceleration =
        DesiredAcceleration.GetClampedToMaxSize(
            Acceleration
        );

    /*
     * Force:
     *
     *     F = ma
     */

    const float Mass =
        FMath::Max(
            Rigidbody->Mass,
            0.001f
        );

    const FVector Force =
        DesiredAcceleration * Mass;

    Rigidbody->AddForce(
        Force
    );


    // ------------------------------------------------------------
    // ANGULAR MOTION
    // ------------------------------------------------------------

    if (!Motion->DesiredAngularVelocity.IsNearlyZero())
    {
        const FVector CurrentAngularVelocity =
            Rigidbody->AngularVelocity;

        /*
         * Motion uses degrees/sec.
         * Chaos uses radians/sec.
         */

        const FVector DesiredAngularVelocityRadians =
            FMath::DegreesToRadians(
                Motion->DesiredAngularVelocity
            );

        const FVector AngularError =
            DesiredAngularVelocityRadians -
            CurrentAngularVelocity;

        FVector AngularAcceleration =
            AngularError / DeltaTime;

        /*
         * Angular acceleration is currently unrestricted.
         *
         * You can add a MaxAngularAcceleration setting
         * later if desired.
         */

        Rigidbody->AddTorque(
            AngularAcceleration * Mass
        );
    }
}


// ============================================================================
// JUMP
// ============================================================================

void UPhysicsSystem::ProcessJump(
    URigidbodyComponent* Rigidbody,
    UMotionComponent* Motion)
{
    if (!Rigidbody || !Motion)
    {
        return;
    }

    if (!Motion->bWantsToJump)
    {
        return;
    }

    /*
     * Consume the request regardless of whether
     * the jump succeeds.
     *
     * Movement/input systems must request another jump.
     */

    Motion->bWantsToJump = false;

    if (!Rigidbody->IsSimulatingPhysics())
    {
        return;
    }

    if (!Rigidbody->bIsGrounded)
    {
        return;
    }

    /*
     * Remove downward velocity before applying jump.
     *
     * This gives a consistent jump regardless of tiny
     * downward velocities from gravity/contact resolution.
     */

    FVector Velocity =
        Rigidbody->LinearVelocity;

    if (Velocity.Z < 0.0f)
    {
        Velocity.Z = 0.0f;

        Rigidbody->SetLinearVelocity(
            Velocity
        );
    }

    /*
     * Apply jump impulse.
     */

    Rigidbody->AddImpulse(
        FVector::UpVector * JumpImpulse
    );

    /*
     * We are no longer grounded.
     */

    Rigidbody->ClearGroundedState();
}


// ============================================================================
// GROUNDED DETECTION
// ============================================================================

void UPhysicsSystem::UpdateGroundedState(
    URigidbodyComponent* Rigidbody)
{
    if (!Rigidbody)
    {
        return;
    }

    if (!Rigidbody->HasPhysicsBody())
    {
        Rigidbody->ClearGroundedState();
        return;
    }

    /*
     * Start slightly above the body's current position.
     *
     * This helps avoid immediately starting inside the
     * surface due to numerical precision.
     */

    const FVector Start =
        Rigidbody->PhysicsLocation;

    const FVector End =
        Start -
        FVector(
            0.0f,
            0.0f,
            GroundCheckDistance
        );

    FHitResult Hit;

    if (!Rigidbody->Sweep(
        Start,
        End,
        Hit))
    {
        Rigidbody->ClearGroundedState();
        return;
    }

    if (!Hit.bBlockingHit)
    {
        Rigidbody->ClearGroundedState();
        return;
    }

    /*
     * Determine whether the surface is walkable.
     *
     * Dot product:
     *
     *     1.0 = perfectly horizontal
     *     0.0 = vertical wall
     */

    const float UpDot =
        FVector::DotProduct(
            Hit.ImpactNormal,
            FVector::UpVector
        );

    if (UpDot >= GroundNormalThreshold)
    {
        Rigidbody->SetGrounded(
            true,
            Hit.ImpactNormal
        );
    }
    else
    {
        Rigidbody->ClearGroundedState();
    }
}


// ============================================================================
// PHYSICS STATE
// ============================================================================

void UPhysicsSystem::UpdatePhysicsState(
    URigidbodyComponent* Rigidbody)
{
    if (!Rigidbody)
    {
        return;
    }

    Rigidbody->UpdatePhysicsState();
}


// ============================================================================
// REGISTRY CLEANUP
// ============================================================================

void UPhysicsSystem::CleanupRegistry()
{
    for (int32 Index = Rigidbodies.Num() - 1;
         Index >= 0;
         --Index)
    {
        if (!Rigidbodies[Index].IsValid())
        {
            Rigidbodies.RemoveAtSwap(Index);
        }
    }

    /*
     * Rebuild the set if necessary.
     *
     * The array is the iteration structure.
     * The set is only for fast registration checks.
     */

    if (RigidbodySet.Num() != Rigidbodies.Num())
    {
        RigidbodySet.Reset();

        for (const TWeakObjectPtr<URigidbodyComponent>& Entry :
             Rigidbodies)
        {
            if (Entry.IsValid())
            {
                RigidbodySet.Add(
                    Entry.Get()
                );
            }
        }
    }
}
