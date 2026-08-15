//
// CharacterActor.cpp
//

#include "CharacterActor.h"

#include "../Components/ControlComponent.h"
#include "../Components/StatsComponent.h"
#include "../Components/AnimationRegistryComponent.h"
#include "../Systems/ActorRegistry.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


ACharacterActor::ACharacterActor()
{
    PrimaryActorTick.bCanEverTick = false;


    // ============================================================
    // COMPONENTS
    // ============================================================

    ControlComponent =
        CreateDefaultSubobject<UControlComponent>(
            TEXT("ControlComponent")
        );


    StatsComponent =
        CreateDefaultSubobject<UStatsComponent>(
            TEXT("StatsComponent")
        );
    
    
    AnimationRegistryComponent =
        CreateDefaultSubobject<UAnimationRegistryComponent>(
            TEXT("AnimationRegistryComponent")
        );
    
    // ============================================================
    // ANIMATION REGISTRY
    // ============================================================

    /*
     * Register the standard animation keys available to this
     * character.
     *
     * The gameplay systems only reference EAnimationKey.
     *
     * The actual animation assets can be assigned in the
     * Unreal Editor through the AnimationRegistryComponent.
     */

    if (AnimationRegistryComponent)
    {
        // --------------------------------------------------------
        // Locomotion
        // --------------------------------------------------------
        
        FAnimationDefinition Idle;
        Idle.Key = EAnimationKey::Idle;
        Idle.PlaybackMode = EAnimationPlaybackMode::Loop;
        Idle.PlayRate = 1.0f;
        Idle.LoopCount = -1;
        Idle.BlendInTime = 0.15f;
        Idle.BlendOutTime = 0.15f;
        Idle.Description =
        FText::FromString(TEXT("Character idle animation."));
        
        AnimationRegistryComponent->Animations.Add(Idle);
        
    }

    // ============================================================
    // CHARACTER MOVEMENT
    // ============================================================

    /*
     * CharacterMovementComponent is created by ACharacter.
     *
     * We do NOT store these values as duplicate properties
     * on ACharacterActor.
     *
     * StatsComponent owns gameplay attributes.
     * MovementSystem applies those attributes to this component.
     */

    UCharacterMovementComponent* Movement =
        GetCharacterMovement();

    if (Movement)
    {
        Movement->bOrientRotationToMovement = true;
        Movement->RotationRate = FRotator(
            0.0f,
            720.0f,
            0.0f
        );
    }
    
    GetCharacterMovement()->bOrientRotationToMovement = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}


// ============================================================
// POST INITIALIZE
// ============================================================

void ACharacterActor::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    /*
     * Actor/component registration happens here.
     *
     * At this point the Actor's default components have been
     * initialized and are available.
     */

    if (UWorld* World = GetWorld())
    {
        if (UActorRegistry* Registry =
            World->GetSubsystem<UActorRegistry>())
        {
            Registry->RegisterActor(this);
        }
    }
}


// ============================================================
// END PLAY
// ============================================================

void ACharacterActor::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    /*
     * Remove the Actor from the registry before it is destroyed.
     */

    if (UWorld* World = GetWorld())
    {
        if (UActorRegistry* Registry =
            World->GetSubsystem<UActorRegistry>())
        {
            Registry->UnregisterActor(this);
        }
    }

    Super::EndPlay(EndPlayReason);
}
