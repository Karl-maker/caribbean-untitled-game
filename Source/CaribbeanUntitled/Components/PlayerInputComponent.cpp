#include "PlayerInputComponent.h"
#include "ControlComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

UPlayerInputComponent::UPlayerInputComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerInputComponent::BeginPlay()
{
    Super::BeginPlay();
    Control = GetOwner() ? GetOwner()->FindComponentByClass<UControlComponent>() : nullptr;
}

void UPlayerInputComponent::BindInput(UInputComponent* PlayerInputComponent)
{
    if (!Control || Control->ControlType != EControlType::Player)
    {
        return;
    }

    APawn* OwningPawn = Cast<APawn>(GetOwner());
    if (APlayerController* PC = OwningPawn ? Cast<APlayerController>(OwningPawn->GetController()) : nullptr)
    {
        if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                if (DefaultMappingContext)
                {
                    Subsystem->AddMappingContext(DefaultMappingContext, 0);
                }
            }
        }
    }

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput)
    {
        return;
    }

    if (MoveAction)
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &UPlayerInputComponent::HandleMoveInput);
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &UPlayerInputComponent::HandleMoveInput);
    }
    if (LookAction)
    {
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &UPlayerInputComponent::HandleLookInput);
    }
    if (JumpAction)
    {
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &UPlayerInputComponent::HandleJumpStarted);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &UPlayerInputComponent::HandleJumpCompleted);
    }
    if (SprintAction)
    {
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &UPlayerInputComponent::HandleSprintStarted);
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &UPlayerInputComponent::HandleSprintCompleted);
    }
    if (CrouchAction)
    {
        EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &UPlayerInputComponent::HandleCrouchStarted);
        EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &UPlayerInputComponent::HandleCrouchCompleted);
    }
}

void UPlayerInputComponent::HandleMoveInput(const FInputActionValue& Value)
{
    if (Control) Control->SetMoveIntent(Value.Get<FVector2D>());
}

void UPlayerInputComponent::HandleLookInput(const FInputActionValue& Value)
{
    if (Control) Control->SetLookInput(Value.Get<FVector2D>());
}

void UPlayerInputComponent::HandleJumpStarted(const FInputActionValue& Value)
{
    if (Control) Control->SetJumpIntent(1.0f);
}

void UPlayerInputComponent::HandleJumpCompleted(const FInputActionValue& Value)
{
    if (Control) Control->SetJumpIntent(0.0f);
}

void UPlayerInputComponent::HandleSprintStarted(const FInputActionValue& Value)
{
    if (Control) Control->SetSprintIntent(1.0f);
}

void UPlayerInputComponent::HandleSprintCompleted(const FInputActionValue& Value)
{
    if (Control) Control->SetSprintIntent(0.0f);
}

void UPlayerInputComponent::HandleCrouchStarted(const FInputActionValue& Value)
{
    if (Control) Control->SetCrouchIntent(1.0f);
}

void UPlayerInputComponent::HandleCrouchCompleted(const FInputActionValue& Value)
{
    if (Control) Control->SetCrouchIntent(0.0f);
}
