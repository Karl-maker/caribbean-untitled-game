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

    if (!PC->IsLocalController())
    {
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


    // ============================================================
    // COMBAT INPUTS
    // ============================================================

    // AIM
    if (AimAction)
    {
        EnhancedInput->BindAction(
            AimAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleAimStarted
        );

        EnhancedInput->BindAction(
            AimAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleAimCompleted
        );
    }

    // RELOAD
    if (ReloadAction)
    {
        EnhancedInput->BindAction(
            ReloadAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleReloadStarted
        );

        EnhancedInput->BindAction(
            ReloadAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleReloadCompleted
        );
    }

    // LIGHT ATTACK
    if (LightAttackAction)
    {
        EnhancedInput->BindAction(
            LightAttackAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleLightAttackStarted
        );

        EnhancedInput->BindAction(
            LightAttackAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleLightAttackCompleted
        );
    }

    // HEAVY ATTACK
    if (HeavyAttackAction)
    {
        EnhancedInput->BindAction(
            HeavyAttackAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleHeavyAttackStarted
        );

        EnhancedInput->BindAction(
            HeavyAttackAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleHeavyAttackCompleted
        );
    }

    // BLOCK
    if (BlockAction)
    {
        EnhancedInput->BindAction(
            BlockAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleBlockStarted
        );

        EnhancedInput->BindAction(
            BlockAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleBlockCompleted
        );
    }

    // DODGE
    if (DodgeAction)
    {
        EnhancedInput->BindAction(
            DodgeAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleDodgeStarted
        );

        EnhancedInput->BindAction(
            DodgeAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleDodgeCompleted
        );
    }

    // LOCK ON
    if (LockOnAction)
    {
        EnhancedInput->BindAction(
            LockOnAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleLockOnStarted
        );

        EnhancedInput->BindAction(
            LockOnAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleLockOnCompleted
        );
    }

    // WEAPON ABILITY
    if (WeaponAbilityAction)
    {
        EnhancedInput->BindAction(
            WeaponAbilityAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleWeaponAbilityStarted
        );

        EnhancedInput->BindAction(
            WeaponAbilityAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleWeaponAbilityCompleted
        );
    }

    // WEAPON SWITCH
    if (WeaponSwitchAction)
    {
        EnhancedInput->BindAction(
            WeaponSwitchAction,
            ETriggerEvent::Started,
            this,
            &UPlayerInputComponent::HandleWeaponSwitchStarted
        );

        EnhancedInput->BindAction(
            WeaponSwitchAction,
            ETriggerEvent::Completed,
            this,
            &UPlayerInputComponent::HandleWeaponSwitchCompleted
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

    Control->SetMoveIntent(FVector2D(
        Control->GetMoveIntent().X,
        Value.Get<bool>() ? 1.0f : 0.0f
    ));
}


void UPlayerInputComponent::HandleMoveBackward(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->SetMoveIntent(FVector2D(
        Control->GetMoveIntent().X,
        Value.Get<bool>() ? -1.0f : 0.0f
    ));
}


void UPlayerInputComponent::HandleMoveRight(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->SetMoveIntent(FVector2D(
        Value.Get<bool>() ? 1.0f : 0.0f,
        Control->GetMoveIntent().Y
    ));
}


void UPlayerInputComponent::HandleMoveLeft(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->SetMoveIntent(FVector2D(
        Value.Get<bool>() ? -1.0f : 0.0f,
        Control->GetMoveIntent().Y
    ));
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
    const float Sensitivity = 2.5f; // Lower = less twitchy look input
    FVector2D LookValue = Value.Get<FVector2D>();
    LookValue *= Sensitivity;
    
    Control->SetLookInput(LookValue);
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


// ============================================================================
// COMBAT INPUT HANDLERS
// ============================================================================

// AIM
void UPlayerInputComponent::HandleAimStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->AimIntent = 1.0f;
}

void UPlayerInputComponent::HandleAimCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->AimIntent = 0.0f;
}

// RELOAD
void UPlayerInputComponent::HandleReloadStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->ReloadIntent = 1.0f;
}

void UPlayerInputComponent::HandleReloadCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->ReloadIntent = 0.0f;
}

// LIGHT ATTACK
void UPlayerInputComponent::HandleLightAttackStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("PlayerInputComponent: Light attack started on %s"),
        *GetOwner()->GetName()
    );

    Control->LightAttackIntent = Value.Get<bool>() ? 1.0f : 0.0f;
}

void UPlayerInputComponent::HandleLightAttackCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->LightAttackIntent = 0.0f;
}

// HEAVY ATTACK
void UPlayerInputComponent::HandleHeavyAttackStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->HeavyAttackIntent = 1.0f;
}

void UPlayerInputComponent::HandleHeavyAttackCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->HeavyAttackIntent = 0.0f;
}

// BLOCK
void UPlayerInputComponent::HandleBlockStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->BlockIntent = 1.0f;
}

void UPlayerInputComponent::HandleBlockCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->BlockIntent = 0.0f;
}

// DODGE
void UPlayerInputComponent::HandleDodgeStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->DodgeIntent = 1.0f;
}

void UPlayerInputComponent::HandleDodgeCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->DodgeIntent = 0.0f;
}

// LOCK ON
void UPlayerInputComponent::HandleLockOnStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->LockOnIntent = 1.0f;
}

void UPlayerInputComponent::HandleLockOnCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->LockOnIntent = 0.0f;
}

// WEAPON ABILITY
void UPlayerInputComponent::HandleWeaponAbilityStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->WeaponAbilityIntent = 1.0f;
}

void UPlayerInputComponent::HandleWeaponAbilityCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->WeaponAbilityIntent = 0.0f;
}

// WEAPON SWITCH
void UPlayerInputComponent::HandleWeaponSwitchStarted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->WeaponSwitchIntent = 1.0f;
}

void UPlayerInputComponent::HandleWeaponSwitchCompleted(
    const FInputActionValue& Value
)
{
    if (!Control)
    {
        return;
    }

    Control->WeaponSwitchIntent = 0.0f;
}
