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
class UActorRegistry;
class UAnimInstance;

UCLASS(Config=Game)
class CARIBBEANUNTITLED_API ACharacterActor : public ACharacter
{
    GENERATED_BODY()

public:

    ACharacterActor();


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
        EditDefaultsOnly,
        ReplicatedUsing=OnRep_CharacterVisualProfileKey,
        Category="Actor|Assets"
    )
    FName CharacterVisualProfileKey = TEXT("DEMO_FEMALE");

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        ReplicatedUsing=OnRep_CharacterVisualProfileKey,
        Category="Actor|Assets"
    )
    FName ReplicatedCharacterVisualProfileKey = TEXT("DEMO_FEMALE");

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UFUNCTION()
    void OnRep_CharacterVisualProfileKey();

    void ApplyCharacterVisualProfile();
};
