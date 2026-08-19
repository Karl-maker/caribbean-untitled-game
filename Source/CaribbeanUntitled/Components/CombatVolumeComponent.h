#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatVolumeComponent.generated.h"

class UShapeComponent;

UENUM(BlueprintType)
enum class ECombatVolumeOwnerType : uint8
{
    Body,
    Weapon,
    Shield,
    Other
};

UENUM(BlueprintType)
enum class EBodyPart : uint8
{
    None,

    Head,
    Torso,

    LeftArm,
    RightArm,

    LeftHand,
    RightHand,

    LeftLeg,
    RightLeg,

    LeftFoot,
    RightFoot
};

UENUM(BlueprintType)
enum class ECombatVolumeShape : uint8
{
    Box,
    Sphere,
    Capsule
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UCombatVolumeComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UCombatVolumeComponent();

protected:

    virtual void OnRegister() override;
    virtual void OnUnregister() override;

public:

    // ---------------------------------------------------------
    // Identity
    // ---------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Volume")
    ECombatVolumeOwnerType OwnerType = ECombatVolumeOwnerType::Body;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Volume")
    EBodyPart BodyPart = EBodyPart::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Volume")
    bool bAttackActive = false;


    // ---------------------------------------------------------
    // Shape
    // ---------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Volume|Shape")
    ECombatVolumeShape ShapeType = ECombatVolumeShape::Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Volume|Shape")
    FVector BoxExtent = FVector(10.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Volume|Shape")
    float SphereRadius = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Volume|Shape")
    float CapsuleRadius = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Volume|Shape")
    float CapsuleHalfHeight = 20.0f;


    // ---------------------------------------------------------
    // Collision
    // ---------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Volume|Collision")
    bool bStartEnabled = true;

    /**
     * The actual UE collision primitive used by this volume.
     *
     * This will be a UBoxComponent, USphereComponent,
     * or UCapsuleComponent depending on ShapeType.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat Volume|Collision")
    TObjectPtr<UShapeComponent> CollisionShape;


    // ---------------------------------------------------------
    // Attachment
    // ---------------------------------------------------------

    /**
     * Attach the combat volume to another scene component.
     *
     * If SocketName is provided, the volume is attached to
     * that socket on the parent component.
     */
    UFUNCTION(BlueprintCallable, Category="Combat Volume|Attachment")
    bool AttachToComponent(
        USceneComponent* Parent,
        FName SocketName = NAME_None
    );

    /**
     * Detach the combat volume from its current parent while
     * keeping its current world transform.
     */
    UFUNCTION(BlueprintCallable, Category="Combat Volume|Attachment")
    void Detach();


    // ---------------------------------------------------------
    // Enable / Disable
    // ---------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category="Combat Volume|Collision")
    void SetEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Combat Volume|Collision")
    bool IsEnabled() const;

    UFUNCTION(BlueprintCallable, Category="Combat Volume")
    void SetAttackActive(bool bActive);

    UFUNCTION(BlueprintPure, Category="Combat Volume")
    bool IsAttackActive() const;

    // ---------------------------------------------------------
    // Access
    // ---------------------------------------------------------

    UFUNCTION(BlueprintPure, Category="Combat Volume")
    UShapeComponent* GetCollisionShape() const
    {
        return CollisionShape;
    }
    
    UFUNCTION(BlueprintPure, Category="Combat Volume|Shape")
    ECombatVolumeShape GetShapeType() const
    {
        return ShapeType;
    }

    UFUNCTION(BlueprintPure, Category="Combat Volume|Shape")
    FVector GetBoxExtent() const
    {
        return BoxExtent;
    }

    UFUNCTION(BlueprintPure, Category="Combat Volume|Shape")
    float GetSphereRadius() const
    {
        return SphereRadius;
    }

    UFUNCTION(BlueprintPure, Category="Combat Volume|Shape")
    float GetCapsuleRadius() const
    {
        return CapsuleRadius;
    }

    UFUNCTION(BlueprintPure, Category="Combat Volume|Shape")
    float GetCapsuleHalfHeight() const
    {
        return CapsuleHalfHeight;
    }

private:

    void CreateCollisionShape();
    void ConfigureCollisionShape();
};
