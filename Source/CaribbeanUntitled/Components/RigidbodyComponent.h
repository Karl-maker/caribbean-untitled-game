#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RigidbodyComponent.generated.h"

class UPrimitiveComponent;

UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API URigidbodyComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    URigidbodyComponent();

    virtual void BeginPlay() override;

    // ========================================================================
    // SIMULATION CONFIGURATION
    // ========================================================================

    /**
     * Whether this actor should be simulated by Chaos.
     *
     * If false, the PhysicsSystem can still use this component for
     * collision queries / kinematic movement depending on configuration.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics|Simulation")
    bool bSimulatePhysics = false;

    /**
     * Whether Chaos gravity should affect this body.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics|Simulation")
    bool bEnableGravity = true;

    /**
     * Mass in kilograms.
     *
     * Chaos uses the actual mass of the physics body. This value is applied
     * to the underlying primitive when possible.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics|Simulation",
        meta=(ClampMin="0.001"))
    float Mass = 80.0f;

    /**
     * Whether this body can generate/query collision.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics|Simulation")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled =
        ECollisionEnabled::QueryAndPhysics;

    /**
     * Collision channel used by the physics body.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics|Collision")
    TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Pawn;

    /**
     * Whether this body should participate in collision.
     *
     * Useful for objects that are simulated but temporarily ignore
     * collision without destroying the physics state.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics|Collision")
    bool bEnableCollision = true;


    // ========================================================================
    // PHYSICS STATE
    // ========================================================================

    /**
     * Whether the body was detected as grounded during the most recent
     * physics update.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Physics|State")
    bool bIsGrounded = false;

    /**
     * Surface normal from the most recent grounded contact.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Physics|State")
    FVector GroundNormal = FVector::UpVector;

    /**
     * Current world-space linear velocity reported by Chaos.
     *
     * This is READ-ONLY physics state.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Physics|State")
    FVector LinearVelocity = FVector::ZeroVector;

    /**
     * Current angular velocity in radians/sec reported by Chaos.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Physics|State")
    FVector AngularVelocity = FVector::ZeroVector;

    /**
     * Current world-space physical location.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Physics|State")
    FVector PhysicsLocation = FVector::ZeroVector;

    /**
     * Current physical rotation.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Physics|State")
    FRotator PhysicsRotation = FRotator::ZeroRotator;


    // ========================================================================
    // FORCE / IMPULSE REQUESTS
    // ========================================================================

    /**
     * Adds a continuous force request.
     *
     * PhysicsSystem consumes this and passes it to Chaos.
     */
    void AddForce(const FVector& Force);

    /**
     * Adds an instantaneous linear impulse.
     */
    void AddImpulse(const FVector& Impulse);

    /**
     * Adds a continuous torque request.
     */
    void AddTorque(const FVector& Torque);

    /**
     * Adds an instantaneous angular impulse.
     */
    void AddAngularImpulse(const FVector& Impulse);


    // ========================================================================
    // CHAOS BODY
    // ========================================================================

    /**
     * Returns the primitive component actually simulated by Chaos.
     */
    UPrimitiveComponent* GetPhysicsBody() const;

    /**
     * Returns whether this component currently has a valid Chaos body.
     */
    bool HasPhysicsBody() const;

    /**
     * Returns whether Chaos simulation is currently active.
     */
    bool IsSimulatingPhysics() const;


    // ========================================================================
    // PHYSICS STATE SYNCHRONIZATION
    // ========================================================================

    /**
     * Reads the current state from Chaos into this component.
     *
     * PhysicsSystem should call this after physics has been simulated.
     */
    void UpdatePhysicsState();

    /**
     * Updates grounded state from a collision/contact result.
     */
    void SetGrounded(bool bGrounded, const FVector& Normal);

    /**
     * Clears grounded state for the next physics update.
     */
    void ClearGroundedState();


    // ========================================================================
    // COLLISION QUERIES
    // ========================================================================

    /**
     * Sweeps this body's collision shape from Start to End.
     *
     * PhysicsSystem can use this for custom queries such as ground checks,
     * movement prediction, and environment tests.
     */
    bool Sweep(
        const FVector& Start,
        const FVector& End,
        FHitResult& Hit
    ) const;


    // ========================================================================
    // FORCE CONSUMPTION
    // ========================================================================

    /**
     * Applies all pending force/impulse requests to Chaos and clears them.
     *
     * Called by PhysicsSystem once per physics update.
     */
    void ConsumePendingForces();


    // ========================================================================
    // PHYSICS CONTROL
    // ========================================================================

    /**
     * Enables/disables Chaos simulation.
     */
    void SetSimulatePhysics(bool bSimulate);

    /**
     * Enables/disables gravity on the underlying physics body.
     */
    void SetEnableGravity(bool bEnable);

    /**
     * Sets the physical linear velocity through Chaos.
     *
     * This should be used sparingly. Prefer forces/impulses when appropriate.
     */
    void SetLinearVelocity(const FVector& Velocity);

    /**
     * Sets the physical angular velocity through Chaos.
     */
    void SetAngularVelocity(const FVector& Velocity);


private:

    // ========================================================================
    // CHAOS BODY
    // ========================================================================

    /**
     * Primitive component owned by the Actor that provides the actual
     * collision/physics representation.
     *
     * Examples:
     *
     * UStaticMeshComponent
     * USkeletalMeshComponent
     * UCapsuleComponent
     * UBoxComponent
     * USphereComponent
     */
    UPROPERTY()
    TObjectPtr<UPrimitiveComponent> PhysicsBody = nullptr;


    // ========================================================================
    // PENDING PHYSICS REQUESTS
    // ========================================================================

    FVector PendingForce = FVector::ZeroVector;

    FVector PendingImpulse = FVector::ZeroVector;

    FVector PendingTorque = FVector::ZeroVector;

    FVector PendingAngularImpulse = FVector::ZeroVector;


    // ========================================================================
    // INTERNAL SETUP
    // ========================================================================

    /**
     * Finds the appropriate primitive component on the owning Actor.
     */
    void ResolvePhysicsBody();

    /**
     * Applies this component's configuration to the Chaos body.
     */
    void ConfigurePhysicsBody();
};
