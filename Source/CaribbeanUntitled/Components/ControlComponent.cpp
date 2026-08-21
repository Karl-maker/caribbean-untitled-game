//
//  ControlComponent.cpp
//  CaribbeanUntitled
//
//  Created by Karl-Johan Bailey on 8/12/26.
//

#include "ControlComponent.h"
#include "../Actors/K98Actor.h"
// Add this include at the top
#include "Net/UnrealNetwork.h"
#include "../EventBus/EventBusSubsystem.h"
#include "CombatComponent.h"
#include "../Events/MovementPayload.h"

UControlComponent::UControlComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    SetIsReplicatedByDefault(true);
}

void UControlComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        ViewYaw = Owner->GetActorRotation().Yaw;
    }
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

    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        ServerSetMoveIntent(MoveIntent);
    }
}

void UControlComponent::ServerSetMoveIntent_Implementation(FVector2D Intent)
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

void UControlComponent::SetJumpIntent(float Weight)
{
    JumpIntent = FMath::Clamp(Weight, 0.0f, 1.0f);

    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        ServerSetJumpIntent(JumpIntent);
    }
}

void UControlComponent::SetInteractIntent(float Weight)  { InteractIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetSprintIntent(float Weight)
{
    SprintIntent = FMath::Clamp(Weight, 0.0f, 1.0f);

    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        ServerSetSprintIntent(SprintIntent);
    }
}
void UControlComponent::SetCrouchIntent(float Weight)    { CrouchIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetDashIntent(float Weight)      { DashIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetAimIntent(float Weight)       { AimIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetFireIntent(float Weight)      { FireIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetReloadIntent(float Weight)    { ReloadIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }

//=========================================================================
// Individual Combat Setters
//=========================================================================

void UControlComponent::SetLightAttackIntent(float Weight)
{
    LightAttackIntent = FMath::Clamp(Weight, 0.0f, 1.0f);

    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        ServerSetLightAttackIntent(LightAttackIntent);
    }
}

void UControlComponent::SetHeavyAttackIntent(float Weight)
{
    HeavyAttackIntent = FMath::Clamp(Weight, 0.0f, 1.0f);

    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        ServerSetHeavyAttackIntent(HeavyAttackIntent);
    }
}
void UControlComponent::RequestLightAttack()
{
    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        ServerRequestLightAttack();
        return;
    }

    if (AActor* Owner = GetOwner())
    {
        if (UCombatComponent* Combat = Owner->FindComponentByClass<UCombatComponent>())
        {
            UE_LOG(LogTemp, Log, TEXT("ControlComponent: RequestLightAttack on server for %s"), *GetNameSafe(Owner));
            Combat->RequestAttack(ECombatAction::LightAttack);
        }
    }
}

void UControlComponent::ServerRequestLightAttack_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("ControlComponent: ServerRequestLightAttack on %s"), *GetNameSafe(GetOwner()));
    RequestLightAttack();
}

void UControlComponent::RequestHeavyAttack()
{
    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        ServerRequestHeavyAttack();
        return;
    }

    if (AActor* Owner = GetOwner())
    {
        if (UCombatComponent* Combat = Owner->FindComponentByClass<UCombatComponent>())
        {
            UE_LOG(LogTemp, Log, TEXT("ControlComponent: RequestHeavyAttack on server for %s"), *GetNameSafe(Owner));
            Combat->RequestAttack(ECombatAction::HeavyAttack);
        }
    }
}

void UControlComponent::ServerRequestHeavyAttack_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("ControlComponent: ServerRequestHeavyAttack on %s"), *GetNameSafe(GetOwner()));
    RequestHeavyAttack();
}
void UControlComponent::SetBlockIntent(float Weight)         { BlockIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetDodgeIntent(float Weight)         { DodgeIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetLockOnIntent(float Weight)        { LockOnIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetWeaponAbilityIntent(float Weight) { WeaponAbilityIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }
void UControlComponent::SetWeaponSwitchIntent(float Weight)  { WeaponSwitchIntent = FMath::Clamp(Weight, 0.0f, 1.0f); }

void UControlComponent::ServerSetJumpIntent_Implementation(float Weight)
{
    JumpIntent = FMath::Clamp(Weight, 0.0f, 1.0f);
}

void UControlComponent::ServerSetSprintIntent_Implementation(float Weight)
{
    SprintIntent = FMath::Clamp(Weight, 0.0f, 1.0f);
}

void UControlComponent::ServerSetLightAttackIntent_Implementation(float Weight)
{
    LightAttackIntent = FMath::Clamp(Weight, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("ControlComponent: ServerSetLightAttackIntent = %f on %s"), LightAttackIntent, *GetNameSafe(GetOwner()));
}

void UControlComponent::ServerSetHeavyAttackIntent_Implementation(float Weight)
{
    HeavyAttackIntent = FMath::Clamp(Weight, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("ControlComponent: ServerSetHeavyAttackIntent = %f on %s"), HeavyAttackIntent, *GetNameSafe(GetOwner()));
}

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

        case EControlIntent::LightAttack:   return LightAttackIntent;
        case EControlIntent::HeavyAttack:   return HeavyAttackIntent;
        case EControlIntent::Block:         return BlockIntent;
        case EControlIntent::Dodge:         return DodgeIntent;
        case EControlIntent::LockOn:        return LockOnIntent;
        case EControlIntent::WeaponAbility: return WeaponAbilityIntent;
        case EControlIntent::WeaponSwitch:  return WeaponSwitchIntent;

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

        case EControlIntent::LightAttack:   LightAttackIntent = ClampedWeight;   break;
        case EControlIntent::HeavyAttack:   HeavyAttackIntent = ClampedWeight;   break;
        case EControlIntent::Block:         BlockIntent = ClampedWeight;         break;
        case EControlIntent::Dodge:         DodgeIntent = ClampedWeight;         break;
        case EControlIntent::LockOn:        LockOnIntent = ClampedWeight;        break;
        case EControlIntent::WeaponAbility: WeaponAbilityIntent = ClampedWeight; break;
        case EControlIntent::WeaponSwitch:  WeaponSwitchIntent = ClampedWeight;  break;

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
// Combat Query Functions
//=========================================================================

bool UControlComponent::WantsToLightAttack() const { return LightAttackIntent > 0.01f; }
bool UControlComponent::WantsToHeavyAttack() const { return HeavyAttackIntent > 0.01f; }
bool UControlComponent::WantsToBlock() const       { return BlockIntent > 0.01f; }
bool UControlComponent::WantsToDodge() const       { return DodgeIntent > 0.01f; }
bool UControlComponent::WantsToLockOn() const      { return LockOnIntent > 0.01f; }
bool UControlComponent::WantsToUseWeaponAbility() const { return WeaponAbilityIntent > 0.01f; }
bool UControlComponent::WantsToSwitchWeapon() const { return WeaponSwitchIntent > 0.01f; }

bool UControlComponent::HasCombatIntent() const
{
    return LightAttackIntent > 0.0f ||
           HeavyAttackIntent > 0.0f ||
           BlockIntent > 0.0f ||
           DodgeIntent > 0.0f ||
           LockOnIntent > 0.0f ||
           WeaponAbilityIntent > 0.0f ||
           WeaponSwitchIntent > 0.0f;
}

//=========================================================================
// Look Functions
//=========================================================================

void UControlComponent::SetLookInput(const FVector2D& Input)
{
    LookInput = Input;

    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        ServerSetLookInput(Input);
    }
}

void UControlComponent::SetLookDirection(const FVector& Direction)
{
    LookDirection = Direction.GetSafeNormal();
}

void UControlComponent::ClearLookInput()
{
    LookInput = FVector2D::ZeroVector;

    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        ServerSetLookInput(FVector2D::ZeroVector);
    }
}

void UControlComponent::SetViewYaw(float NewYaw)
{
    ViewYaw = FRotator::NormalizeAxis(NewYaw);

    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        ServerSetViewYaw(ViewYaw);
    }
}

void UControlComponent::ServerSetViewYaw_Implementation(float NewYaw)
{
    ViewYaw = FRotator::NormalizeAxis(NewYaw);
}

void UControlComponent::ServerSetLookInput_Implementation(FVector2D Input)
{
    LookInput = Input.ClampAxes(-1.0f, 1.0f);
}

void UControlComponent::SetReplicatedLocomotion(ELocomotion NewLocomotion)
{
    if (ReplicatedLocomotion == NewLocomotion)
    {
        return;
    }

    ReplicatedLocomotion = NewLocomotion;

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        OnRep_ReplicatedLocomotion();
    }
}

//=========================================================================
// Clearing Functions
//=========================================================================

void UControlComponent::ClearIntents()
{
    ClearMovementIntent();
    ClearActionIntent();
    ClearCombatIntent();
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

void UControlComponent::ClearCombatIntent()
{
    LightAttackIntent = 0.0f;
    HeavyAttackIntent = 0.0f;
    BlockIntent = 0.0f;
    DodgeIntent = 0.0f;
    LockOnIntent = 0.0f;
    WeaponAbilityIntent = 0.0f;
    WeaponSwitchIntent = 0.0f;
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

void UControlComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate movement intent
    DOREPLIFETIME(UControlComponent, MoveIntent);
    DOREPLIFETIME(UControlComponent, ReplicatedLocomotion);
    
    // Replicate action intents
    DOREPLIFETIME(UControlComponent, JumpIntent);
    DOREPLIFETIME(UControlComponent, InteractIntent);
    DOREPLIFETIME(UControlComponent, SprintIntent);
    DOREPLIFETIME(UControlComponent, CrouchIntent);
    DOREPLIFETIME(UControlComponent, DashIntent);
    DOREPLIFETIME(UControlComponent, AimIntent);
    DOREPLIFETIME(UControlComponent, FireIntent);
    DOREPLIFETIME(UControlComponent, ReloadIntent);
    
    // Replicate combat intents
    DOREPLIFETIME(UControlComponent, LightAttackIntent);
    DOREPLIFETIME(UControlComponent, HeavyAttackIntent);
    DOREPLIFETIME(UControlComponent, BlockIntent);
    DOREPLIFETIME(UControlComponent, DodgeIntent);
    DOREPLIFETIME(UControlComponent, LockOnIntent);
    DOREPLIFETIME(UControlComponent, WeaponAbilityIntent);
    DOREPLIFETIME(UControlComponent, WeaponSwitchIntent);
    
}

void UControlComponent::OnRep_ReplicatedLocomotion()
{
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = GetWorld())
        {
            if (UEventBusSubsystem* EventBus = World->GetSubsystem<UEventBusSubsystem>())
            {
                FMovementPayload Payload;
                Payload.Source = Owner;
                Payload.Type = ReplicatedLocomotion;
                EventBus->Publish(Payload);
            }
        }
    }
}
