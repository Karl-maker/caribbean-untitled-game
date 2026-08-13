//
// PhysicsComponent.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RigidbodyComponent.generated.h"

class URigidbodyComponent;

UENUM(BlueprintType)
enum class EPhysicsShape : uint8
{
    Box,
    Capsule,
    Sphere,
    Mesh
};

UCLASS(ClassGroup=(Gameplay), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API URigidbodyComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    URigidbodyComponent();

    virtual void BeginPlay() override;

    // ------------------------------------------------------------
    // PHYSICS CONFIGURATION
    // ------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics")
    bool bSimulatePhysics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics")
    float Mass = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics")
    EPhysicsShape CollisionShape = EPhysicsShape::Capsule;

    // ------------------------------------------------------------
    // COLLISION
    // ------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled =
        ECollisionEnabled::QueryAndPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
    TEnumAsByte<ECollisionChannel> CollisionChannel =
        ECC_Pawn;

    // ------------------------------------------------------------
    // PHYSICS STATE
    // ------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Physics")
    bool bIsGrounded = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Physics")
    FVector GroundNormal = FVector::UpVector;

    // ------------------------------------------------------------
    // FORCES / IMPULSES
    // ------------------------------------------------------------

    void AddForce(const FVector& Force);

    void AddImpulse(const FVector& Impulse);

    void AddTorque(const FVector& Torque);

    void AddAngularImpulse(const FVector& Impulse);

    // ------------------------------------------------------------
    // COLLISION QUERIES
    // ------------------------------------------------------------

    bool Sweep(
        const FVector& Start,
        const FVector& End,
        FHitResult& Hit
    ) const;

    // ------------------------------------------------------------
    // PHYSICS STATE
    // ------------------------------------------------------------

    bool IsSimulatingPhysics() const;

    UPrimitiveComponent* GetPhysicsBody() const;

private:

    UPROPERTY()
    TObjectPtr<UPrimitiveComponent> PhysicsBody;

    FVector PendingForce = FVector::ZeroVector;
    FVector PendingImpulse = FVector::ZeroVector;
    FVector PendingTorque = FVector::ZeroVector;
    FVector PendingAngularImpulse = FVector::ZeroVector;

public:

    /**
     * Called by PhysicsSystem.
     *
     * Applies forces/impulses accumulated during the frame.
     */
    void ConsumePendingForces();
};
