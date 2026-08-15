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
};
