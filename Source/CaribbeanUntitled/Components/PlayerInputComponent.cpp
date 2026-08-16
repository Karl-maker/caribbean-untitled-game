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

    Control =
        GetOwner()
        ? GetOwner()->FindComponentByClass<UControlComponent>()
        : nullptr;
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
            ETriggerEvent::Triggered,
            this,
            &UPlayerInputComponent::HandleLookInput
        );

        EnhancedInput->BindAction(
            LookAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleLookInput
        );
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
// MOVEMENT - DISCRETE WASD HANDLERS
// ============================================================================

void UPlayerInputComponent::HandleMoveForward(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    const bool bIsPressed = Value.Get<bool>();
    FVector2D CurrentIntent = Control->MoveIntent;
    
    // Y = +1.0 is Forward
    if (bIsPressed)
    {
        CurrentIntent.Y = 1.0f;
    }
    else
    {
        // Only clear forward if we're not going backward
        if (CurrentIntent.Y > 0.0f)
        {
            CurrentIntent.Y = 0.0f;
        }
    }
    
    Control->SetMoveIntent(CurrentIntent);
}


void UPlayerInputComponent::HandleMoveBackward(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    const bool bIsPressed = Value.Get<bool>();
    FVector2D CurrentIntent = Control->MoveIntent;
    
    // Y = -1.0 is Backward
    if (bIsPressed)
    {
        CurrentIntent.Y = -1.0f;
    }
    else
    {
        // Only clear backward if we're not going forward
        if (CurrentIntent.Y < 0.0f)
        {
            CurrentIntent.Y = 0.0f;
        }
    }
    
    Control->SetMoveIntent(CurrentIntent);
}


void UPlayerInputComponent::HandleMoveRight(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    const bool bIsPressed = Value.Get<bool>();
    FVector2D CurrentIntent = Control->MoveIntent;
    
    // X = +1.0 is Right
    if (bIsPressed)
    {
        CurrentIntent.X = 1.0f;
    }
    else
    {
        // Only clear right if we're not going left
        if (CurrentIntent.X > 0.0f)
        {
            CurrentIntent.X = 0.0f;
        }
    }
    
    Control->SetMoveIntent(CurrentIntent);
}


void UPlayerInputComponent::HandleMoveLeft(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    const bool bIsPressed = Value.Get<bool>();
    FVector2D CurrentIntent = Control->MoveIntent;
    
    // X = -1.0 is Left
    if (bIsPressed)
    {
        CurrentIntent.X = -1.0f;
    }
    else
    {
        // Only clear left if we're not going right
        if (CurrentIntent.X < 0.0f)
        {
            CurrentIntent.X = 0.0f;
        }
    }
    
    Control->SetMoveIntent(CurrentIntent);
}


// ============================================================================
// LOOK
// ============================================================================

void UPlayerInputComponent::HandleLookInput(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->SetLookInput(
        Value.Get<FVector2D>()
    );
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

    Control->SetJumpIntent(1.0f);
}


void UPlayerInputComponent::HandleJumpCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->SetJumpIntent(0.0f);
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

    Control->SetSprintIntent(1.0f);
}


void UPlayerInputComponent::HandleSprintCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->SetSprintIntent(0.0f);
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

    Control->SetCrouchIntent(1.0f);
}


void UPlayerInputComponent::HandleCrouchCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->SetCrouchIntent(0.0f);
}
