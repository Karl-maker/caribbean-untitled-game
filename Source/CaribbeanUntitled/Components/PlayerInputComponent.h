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
};
