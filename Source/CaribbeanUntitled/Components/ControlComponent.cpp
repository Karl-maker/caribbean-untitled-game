//
//  ControlComponent.cpp
//  CaribbeanUntitled
//
//  Created by Karl-Johan Bailey on 8/12/26.
//

#include "ControlComponent.h"
#include "../Actors/K98Actor.h"

UControlComponent::UControlComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UControlComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UControlComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Intentionally empty: WorldMoveDirection is an independent field set
    // directly via SetWorldMoveDirection() / MoveToWorldLocation(). It must
    // persist across ticks until something explicitly changes it, so this
    // component does not (and must not) recompute it from MoveIntent here.
    // MovementSystem derives the player-intent direction on its own.
}

//=========================================================================
// Movement Direction Functions
//=========================================================================

void UControlComponent::SetMoveIntent(const FVector2D& Intent)
{
    MoveIntent = Intent.ClampAxes(-1.0f, 1.0f);
}

void UControlComponent::SetWorldMoveDirection(const FVector& WorldDirection)
{
    WorldMoveDirection = WorldDirection.GetSafeNormal();
}

void UControlComponent::SetForwardBackIntent(float Weight)
{
    MoveIntent.Y = FMath::Clamp(Weight, -1.0f, 1.0f);
}

void UControlComponent::SetLeftRightIntent(float Weight)
{
    MoveIntent.X = FMath::Clamp(Weight, -1.0f, 1.0f);
}

void UControlComponent::SetDesiredWorldLocation(const FVector& Location)
{
    DesiredWorldLocation = Location;
}

void UControlComponent::SetDirectionControlStrength(float Strength)
{
    DirectionControlStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
}

void UControlComponent::SetDirectionAcceptanceRadius(float Radius)
{
    DirectionAcceptanceRadius = FMath::Max(Radius, 0.0f);
}

void UControlComponent::MoveToWorldLocation(const FVector& TargetLocation)
{
    DesiredWorldLocation = TargetLocation;
    bSeekingDestination = true;
    // Direction is now computed fresh every tick by MovementSystem,
    // not here — this just records the request.
}

void UControlComponent::ClearDestination()
{
    bSeekingDestination = false;
    WorldMoveDirection = FVector::ZeroVector;
}

//=========================================================================
// Individual Action Setters
//=========================================================================

void UControlComponent::SetJumpIntent(float Weight)      { JumpIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetInteractIntent(float Weight)  { InteractIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetSprintIntent(float Weight)    { SprintIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetCrouchIntent(float Weight)    { CrouchIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetDashIntent(float Weight)      { DashIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetAimIntent(float Weight)       { AimIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetFireIntent(float Weight)      { FireIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetReloadIntent(float Weight)    { ReloadIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }

//=========================================================================
// Generic Intent Functions
//=========================================================================

float UControlComponent::GetIntentWeight(EControlIntent Intent) const
{
    switch (Intent)
    {
        case EControlIntent::MoveForward:
        case EControlIntent::MoveBack:
            return MoveIntent.Y;

        case EControlIntent::MoveLeft:
        case EControlIntent::MoveRight:
            return MoveIntent.X;

        case EControlIntent::Jump:      return JumpIntent;
        case EControlIntent::Interact:  return InteractIntent;
        case EControlIntent::Sprint:    return SprintIntent;
        case EControlIntent::Crouch:    return CrouchIntent;
        case EControlIntent::Dash:      return DashIntent;
        case EControlIntent::Aim:       return AimIntent;
        case EControlIntent::Fire:      return FireIntent;
        case EControlIntent::Reload:    return ReloadIntent;

        default:
            return 0.0f;
    }
}

void UControlComponent::SetIntentWeight(EControlIntent Intent, float Weight)
{
    const bool bIsMovementIntent =
        (Intent == EControlIntent::MoveForward ||
         Intent == EControlIntent::MoveBack    ||
         Intent == EControlIntent::MoveLeft    ||
         Intent == EControlIntent::MoveRight);

    const float ClampedWeight = bIsMovementIntent
        ? FMath::Clamp(Weight, -1.0f, 1.0f)
        : FMath::Clamp(Weight, 0.0f, 1.0f);

    switch (Intent)
    {
        case EControlIntent::MoveForward: MoveIntent.Y = ClampedWeight;  break;
        case EControlIntent::MoveBack:    MoveIntent.Y = -ClampedWeight; break;
        case EControlIntent::MoveLeft:    MoveIntent.X = -ClampedWeight; break;
        case EControlIntent::MoveRight:   MoveIntent.X = ClampedWeight;  break;

        case EControlIntent::Jump:      JumpIntent = ClampedWeight;     break;
        case EControlIntent::Interact:  InteractIntent = ClampedWeight; break;
        case EControlIntent::Sprint:    SprintIntent = ClampedWeight;   break;
        case EControlIntent::Crouch:    CrouchIntent = ClampedWeight;   break;
        case EControlIntent::Dash:      DashIntent = ClampedWeight;     break;
        case EControlIntent::Aim:       AimIntent = ClampedWeight;      break;
        case EControlIntent::Fire:      FireIntent = ClampedWeight;     break;
        case EControlIntent::Reload:    ReloadIntent = ClampedWeight;   break;

        default:
            break;
    }
}

//=========================================================================
// Intent Query Functions
//=========================================================================

bool UControlComponent::HasIntent(EControlIntent Intent, float MinimumWeight) const
{
    return FMath::Abs(GetIntentWeight(Intent)) >= MinimumWeight;
}

bool UControlComponent::HasMovementIntent() const
{
    return !MoveIntent.IsNearlyZero() || !WorldMoveDirection.IsNearlyZero();
}

bool UControlComponent::HasActionIntent() const
{
    return JumpIntent > 0.0f ||
           InteractIntent > 0.0f ||
           SprintIntent > 0.0f ||
           CrouchIntent > 0.0f ||
           DashIntent > 0.0f ||
           AimIntent > 0.0f ||
           FireIntent > 0.0f ||
           ReloadIntent > 0.0f;
}

bool UControlComponent::WantsToSprint() const { return SprintIntent > 0.01f && HasMovementIntent(); }
bool UControlComponent::WantsToCrouch() const { return CrouchIntent > 0.01f; }
bool UControlComponent::WantsToDash() const   { return DashIntent > 0.01f; }
bool UControlComponent::WantsToAim() const    { return AimIntent > 0.01f; }
bool UControlComponent::WantsToFire() const   { return FireIntent > 0.01f; }

//=========================================================================
// Look Functions
//=========================================================================

void UControlComponent::SetLookInput(const FVector2D& Input)
{
    LookInput = Input;
}

void UControlComponent::SetLookDirection(const FVector& Direction)
{
    LookDirection = Direction.GetSafeNormal();
}

void UControlComponent::ClearLookInput()
{
    LookInput = FVector2D::ZeroVector;
}

//=========================================================================
// Clearing Functions
//=========================================================================

void UControlComponent::ClearIntents()
{
    ClearMovementIntent();
    ClearActionIntent();
    ClearLookInput();
}

void UControlComponent::ClearMovementIntent()
{
    MoveIntent = FVector2D::ZeroVector;
    WorldMoveDirection = FVector::ZeroVector;
    bSeekingDestination = false;
}

void UControlComponent::ClearActionIntent()
{
    JumpIntent = 0.0f;
    InteractIntent = 0.0f;
    SprintIntent = 0.0f;
    CrouchIntent = 0.0f;
    DashIntent = 0.0f;
    AimIntent = 0.0f;
    FireIntent = 0.0f;
    ReloadIntent = 0.0f;
}

void UControlComponent::SetLookAtLocation(const FVector& WorldLocation)
{
    if (AActor* Owner = GetOwner())
    {
        FVector Direction = WorldLocation - Owner->GetActorLocation();
        Direction.Z = 0.0f;

        if (!Direction.IsNearlyZero())
        {
            SetLookDirection(Direction);
        }
    }
}
