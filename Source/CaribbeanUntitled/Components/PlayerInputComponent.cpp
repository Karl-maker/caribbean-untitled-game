#include "PlayerInputComponent.h"

#include "ControlComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/LocalPlayer.h"


UPlayerInputComponent::UPlayerInputComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


void UPlayerInputComponent::BeginPlay()
{
    Super::BeginPlay();

    Control = GetOwner() ? GetOwner()->FindComponentByClass<UControlComponent>() : nullptr;
    
    // Force clear any look direction that might be set
    if (Control)
    {
        Control->SetLookDirection(FVector::ZeroVector);
        Control->ClearLookInput();
    }
}


void UPlayerInputComponent::BindInput(
    UInputComponent* PlayerInputComponent
)
{
    // ============================================================
    // FIND CONTROL COMPONENT
    // ============================================================

    if (!Control)
    {
        Control =
            GetOwner()
            ? GetOwner()->FindComponentByClass<UControlComponent>()
            : nullptr;
    }

    if (!Control)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PlayerInputComponent: No ControlComponent found on %s"),
            *GetOwner()->GetName()
        );

        return;
    }


    // ============================================================
    // ONLY PLAYER CONTROLLED ACTORS ACCEPT PLAYER INPUT
    // ============================================================

    if (Control->ControlType != EControlType::Player)
    {
        return;
    }


    // ============================================================
    // ENHANCED INPUT COMPONENT
    // ============================================================

    UEnhancedInputComponent* EnhancedInput =
        Cast<UEnhancedInputComponent>(PlayerInputComponent);

    if (!EnhancedInput)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PlayerInputComponent: InputComponent is not EnhancedInputComponent")
        );

        return;
    }


    // ============================================================
    // GET PLAYER CONTROLLER
    // ============================================================

    APawn* OwningPawn =
        Cast<APawn>(GetOwner());

    if (!OwningPawn)
    {
        return;
    }


    APlayerController* PC =
        Cast<APlayerController>(
            OwningPawn->GetController()
        );

    if (!PC)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PlayerInputComponent: %s has no PlayerController"),
            *GetOwner()->GetName()
        );

        return;
    }


    // ============================================================
    // ADD MAPPING CONTEXT
    // ============================================================

    if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            LocalPlayer->GetSubsystem<
                UEnhancedInputLocalPlayerSubsystem>())
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(
                    DefaultMappingContext,
                    0
                );
            }
        }
    }


    // ============================================================
    // MOVEMENT - DISCRETE WASD KEYS
    // ============================================================

    // Forward (W) - Y axis: +1 = Forward
    if (MoveForwardAction)
    {
        EnhancedInput->BindAction(
            MoveForwardAction,
            ETriggerEvent::Triggered,
            this,
            &UPlayerInputComponent::HandleMoveForward
        );

        EnhancedInput->BindAction(
            MoveForwardAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleMoveForward
        );
    }

    // Backward (S) - Y axis: -1 = Back
    if (MoveBackwardAction)
    {
        EnhancedInput->BindAction(
            MoveBackwardAction,
            ETriggerEvent::Triggered,
            this,
            &UPlayerInputComponent::HandleMoveBackward
        );

        EnhancedInput->BindAction(
            MoveBackwardAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleMoveBackward
        );
    }

    // Right (D) - X axis: +1 = Right
    if (MoveRightAction)
    {
        EnhancedInput->BindAction(
            MoveRightAction,
            ETriggerEvent::Triggered,
            this,
            &UPlayerInputComponent::HandleMoveRight
        );

        EnhancedInput->BindAction(
            MoveRightAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleMoveRight
        );
    }

    // Left (A) - X axis: -1 = Left
    if (MoveLeftAction)
    {
        EnhancedInput->BindAction(
            MoveLeftAction,
            ETriggerEvent::Triggered,
            this,
            &UPlayerInputComponent::HandleMoveLeft
        );

        EnhancedInput->BindAction(
            MoveLeftAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleMoveLeft
        );
    }


    // ============================================================
    // LOOK
    // ============================================================

    if (LookAction)
    {
        EnhancedInput->BindAction(
            LookAction,
            ETriggerEvent::Triggered,  // Use Triggered for continuous mouse movement
            this,
            &UPlayerInputComponent::HandleLookInput
        );
        
        // Don't bind to Completed for mouse - it won't trigger properly
        // Remove the Completed binding for LookAction
    }


    // ============================================================
    // JUMP
    // ============================================================

    if (JumpAction)
    {
        EnhancedInput->BindAction(
            JumpAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleJumpStarted
        );

        EnhancedInput->BindAction(
            JumpAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleJumpCompleted
        );
    }


    // ============================================================
    // SPRINT
    // ============================================================

    if (SprintAction)
    {
        EnhancedInput->BindAction(
            SprintAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleSprintStarted
        );

        EnhancedInput->BindAction(
            SprintAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleSprintCompleted
        );
    }


    // ============================================================
    // CROUCH
    // ============================================================

    if (CrouchAction)
    {
        EnhancedInput->BindAction(
            CrouchAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleCrouchStarted
        );

        EnhancedInput->BindAction(
            CrouchAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleCrouchCompleted
        );
    }
}

// ============================================================================
// MOVEMENT
// ============================================================================

void UPlayerInputComponent::HandleMoveForward(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->MoveIntent.Y =
        Value.Get<bool>() ? 1.0f : 0.0f;
}


void UPlayerInputComponent::HandleMoveBackward(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->MoveIntent.Y =
        Value.Get<bool>() ? -1.0f : 0.0f;
}


void UPlayerInputComponent::HandleMoveRight(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->MoveIntent.X =
        Value.Get<bool>() ? 1.0f : 0.0f;
}


void UPlayerInputComponent::HandleMoveLeft(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }
    

    Control->MoveIntent.X =
        Value.Get<bool>() ? -1.0f : 0.0f;
}


// ============================================================================
// LOOK
// ============================================================================
void UPlayerInputComponent::HandleLookInput(const FInputActionValue& Value)
{
    if (!Control)
    {
        return;
    }

    // Apply sensitivity scaling
    const float Sensitivity = 2.0f; // Adjust this value to taste
    FVector2D LookValue = Value.Get<FVector2D>();
    LookValue *= Sensitivity;
    
    Control->LookInput = LookValue;
    
    UE_LOG(LogTemp, Warning, TEXT("Look Input (Scaled): X=%f, Y=%f"),
           Control->LookInput.X, Control->LookInput.Y);
}


// ============================================================================
// JUMP
// ============================================================================

void UPlayerInputComponent::HandleJumpStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->JumpIntent = 1.0f;
}


void UPlayerInputComponent::HandleJumpCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->JumpIntent = 0.0f;
}
 
// ============================================================================
// SPRINT
// ============================================================================

void UPlayerInputComponent::HandleSprintStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->SprintIntent = 1.0f;
}


void UPlayerInputComponent::HandleSprintCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->SprintIntent = 0.0f;
}


// ============================================================================
// CROUCH
// ============================================================================

void UPlayerInputComponent::HandleCrouchStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->CrouchIntent = 1.0f;
}


void UPlayerInputComponent::HandleCrouchCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->CrouchIntent = 0.0f;
}
