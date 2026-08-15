#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "PlayerInputComponent.generated.h"

class UControlComponent;
class UInputMappingContext;
class UInputAction;
class UInputComponent;

/**
 * Binds Enhanced Input actions to a Player-controlled Actor's
 * UControlComponent. Only sets up bindings if the sibling
 * ControlComponent has ControlType == Player.
 *
 * This component does not perform movement or gameplay logic —
 * it only translates raw input into control intent, same division
 * of responsibility as ControlComponent -> MovementSystem.
 */
UCLASS(ClassGroup=(Input), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UPlayerInputComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerInputComponent();

    /** Called by the owning Pawn's SetupPlayerInputComponent. */
    void BindInput(UInputComponent* PlayerInputComponent);

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> CrouchAction;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<UControlComponent> Control;

    void HandleMoveInput(const FInputActionValue& Value);
    void HandleLookInput(const FInputActionValue& Value);
    void HandleJumpStarted(const FInputActionValue& Value);
    void HandleJumpCompleted(const FInputActionValue& Value);
    void HandleSprintStarted(const FInputActionValue& Value);
    void HandleSprintCompleted(const FInputActionValue& Value);
    void HandleCrouchStarted(const FInputActionValue& Value);
    void HandleCrouchCompleted(const FInputActionValue& Value);
};
