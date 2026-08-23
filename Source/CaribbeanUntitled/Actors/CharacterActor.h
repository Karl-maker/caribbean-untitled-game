//
// CharacterActor.h
//

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Components/AnimationRegistryComponent.h"
#include "CharacterActor.generated.h"

class UControlComponent;
class UStatsComponent;
class UAnimationRegistryComponent;
class UMoveSetComponent;
class UCombatComponent;
class UAttackStateComponent;
class UActorRegistry;
class UAnimInstance;
class UAnimSequenceBase;
class UCombatVolumeComponent;
class UPoseableMeshComponent;
class UAnimationSystem;

UCLASS(Config=Game)
class CARIBBEANUNTITLED_API ACharacterActor : public ACharacter
{
    GENERATED_BODY()

public:

    ACharacterActor();
    virtual void Tick(float DeltaSeconds) override;


protected:

    // ============================================================
    // LIFECYCLE
    // ============================================================

    virtual void PostInitializeComponents() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;


    // ============================================================
    // GAMEPLAY COMPONENTS
    // ============================================================

    /**
     * Handles player/AI movement intent.
     *
     * Examples:
     * - Move
     * - Sprint
     * - Crouch
     * - Jump
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor|Components"
    )
    TObjectPtr<UControlComponent> ControlComponent;

    /**
     * Stores gameplay attributes for this Actor.
     *
     * Examples:
     * - Walk speed
     * - Sprint speed
     * - Crouch speed
     * - Jump strength
     * - Stamina
     * - Dash values
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor|Components"
    )
    TObjectPtr<UStatsComponent> StatsComponent;
    
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor|Components"
    )
    TObjectPtr<UAnimationRegistryComponent> AnimationRegistryComponent;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor|Components"
    )
    TObjectPtr<UMoveSetComponent> MoveSetComponent;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor|Components"
    )
    TObjectPtr<UCombatComponent> CombatComponent;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor|Components"
    )
    TObjectPtr<UAttackStateComponent> AttackStateComponent;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Actor|Components"
    )
    TObjectPtr<UPoseableMeshComponent> PoseableMeshComponent;

    UPROPERTY(
        EditDefaultsOnly,
        ReplicatedUsing=OnRep_CharacterVisualProfileKey,
        Category="Actor|Assets"
    )
    FName CharacterVisualProfileKey = TEXT("DEMO_FEMALE");

    UPROPERTY(
        EditDefaultsOnly,
        Category="Actor|Assets"
    )
    FName CombatAttachmentProfileKey = TEXT("DEMO_FEMALE");

    UPROPERTY(
        EditDefaultsOnly,
        Category="Actor|Animation"
    )
    FName UpperBodyBreakpointBoneName = TEXT("Spine");

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        ReplicatedUsing=OnRep_CharacterVisualProfileKey,
        Category="Actor|Assets"
    )
    FName ReplicatedCharacterVisualProfileKey = TEXT("DEMO_FEMALE");
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCombatVolumeComponent> HeadCombatVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCombatVolumeComponent> TorsoCombatVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCombatVolumeComponent> LeftHandCombatVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCombatVolumeComponent> RightHandCombatVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCombatVolumeComponent> LeftFootCombatVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCombatVolumeComponent> RightFootCombatVolume;

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UFUNCTION()
    void OnRep_CharacterVisualProfileKey();

    void ApplyCharacterVisualProfile();

public:
    void SyncPoseableMeshFromSkeletalMesh();
    void UpdatePoseableMeshFromCurrentAnimation(float DeltaSeconds);
};
