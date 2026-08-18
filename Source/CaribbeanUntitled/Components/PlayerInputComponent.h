#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "PlayerInputComponent.generated.h"

class UControlComponent;
class UInputMappingContext;
class UInputAction;
class UInputComponent;

UCLASS(ClassGroup=(Input), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UPlayerInputComponent
    : public UActorComponent
{
    GENERATED_BODY()

public:

    UPlayerInputComponent();

    /**
     * Called by APlayableActor when Unreal initializes
     * the Pawn's input.
     */
    void BindInput(UInputComponent* PlayerInputComponent);


    // ============================================================
    // ENHANCED INPUT ASSETS
    // ============================================================

    UPROPERTY(EditAnywhere, Category="Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;
    
    UPROPERTY(EditAnywhere, Category="Input")
    TObjectPtr<UInputAction> MoveForwardAction;

    UPROPERTY(EditAnywhere, Category="Input")
    TObjectPtr<UInputAction> MoveBackwardAction;

    UPROPERTY(EditAnywhere, Category="Input")
    TObjectPtr<UInputAction> MoveRightAction;

    UPROPERTY(EditAnywhere, Category="Input")
    TObjectPtr<UInputAction> MoveLeftAction;

    UPROPERTY(EditAnywhere, Category="Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, Category="Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditAnywhere, Category="Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditAnywhere, Category="Input")
    TObjectPtr<UInputAction> CrouchAction;

    // ============================================================
    // COMBAT INPUT ASSETS
    // ============================================================

    UPROPERTY(EditAnywhere, Category="Input|Combat")
    TObjectPtr<UInputAction> AimAction;

    UPROPERTY(EditAnywhere, Category="Input|Combat")
    TObjectPtr<UInputAction> ReloadAction;

    UPROPERTY(EditAnywhere, Category="Input|Combat")
    TObjectPtr<UInputAction> LightAttackAction;

    UPROPERTY(EditAnywhere, Category="Input|Combat")
    TObjectPtr<UInputAction> HeavyAttackAction;

    UPROPERTY(EditAnywhere, Category="Input|Combat")
    TObjectPtr<UInputAction> BlockAction;

    UPROPERTY(EditAnywhere, Category="Input|Combat")
    TObjectPtr<UInputAction> DodgeAction;

    UPROPERTY(EditAnywhere, Category="Input|Combat")
    TObjectPtr<UInputAction> LockOnAction;

    UPROPERTY(EditAnywhere, Category="Input|Combat")
    TObjectPtr<UInputAction> WeaponAbilityAction;

    UPROPERTY(EditAnywhere, Category="Input|Combat")
    TObjectPtr<UInputAction> WeaponSwitchAction;


protected:

    virtual void BeginPlay() override;


private:

    UPROPERTY()
    TObjectPtr<UControlComponent> Control;


    // ============================================================
    // INPUT HANDLERS
    // ============================================================

    void HandleMoveForward(
        const FInputActionValue& Value
    );

    void HandleMoveBackward(
        const FInputActionValue& Value
    );

    void HandleMoveRight(
        const FInputActionValue& Value
    );

    void HandleMoveLeft(
        const FInputActionValue& Value
    );

    void HandleLookInput(
        const FInputActionValue& Value
    );

    void HandleJumpStarted(
        const FInputActionValue& Value
    );

    void HandleJumpCompleted(
        const FInputActionValue& Value
    );

    void HandleSprintStarted(
        const FInputActionValue& Value
    );

    void HandleSprintCompleted(
        const FInputActionValue& Value
    );

    void HandleCrouchStarted(
        const FInputActionValue& Value
    );

    void HandleCrouchCompleted(
        const FInputActionValue& Value
    );

    // ============================================================
    // COMBAT INPUT HANDLERS
    // ============================================================

    void HandleAimStarted(
        const FInputActionValue& Value
    );

    void HandleAimCompleted(
        const FInputActionValue& Value
    );

    void HandleReloadStarted(
        const FInputActionValue& Value
    );

    void HandleReloadCompleted(
        const FInputActionValue& Value
    );

    void HandleLightAttackStarted(
        const FInputActionValue& Value
    );

    void HandleLightAttackCompleted(
        const FInputActionValue& Value
    );

    void HandleHeavyAttackStarted(
        const FInputActionValue& Value
    );

    void HandleHeavyAttackCompleted(
        const FInputActionValue& Value
    );

    void HandleBlockStarted(
        const FInputActionValue& Value
    );

    void HandleBlockCompleted(
        const FInputActionValue& Value
    );

    void HandleDodgeStarted(
        const FInputActionValue& Value
    );

    void HandleDodgeCompleted(
        const FInputActionValue& Value
    );

    void HandleLockOnStarted(
        const FInputActionValue& Value
    );

    void HandleLockOnCompleted(
        const FInputActionValue& Value
    );

    void HandleWeaponAbilityStarted(
        const FInputActionValue& Value
    );

    void HandleWeaponAbilityCompleted(
        const FInputActionValue& Value
    );

    void HandleWeaponSwitchStarted(
        const FInputActionValue& Value
    );

    void HandleWeaponSwitchCompleted(
        const FInputActionValue& Value
    );
};
