//
// CharacterActor.cpp
//

#include "CharacterActor.h"

#include "../Components/ControlComponent.h"
#include "../Components/StatsComponent.h"
#include "../Components/AnimationRegistryComponent.h"
#include "../Systems/ActorRegistry.h"
#include "Components/SkeletalMeshComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


ACharacterActor::ACharacterActor()
{
    PrimaryActorTick.bCanEverTick = false;

    USkeletalMeshComponent* Mesh = GetMesh();
    UCapsuleComponent* Capsule = GetCapsuleComponent();

    if (Mesh && Capsule)
    {
        const FBoxSphereBounds Bounds = Mesh->CalcBounds(
            Mesh->GetComponentTransform()
        );

        const float MeshBottomZ =
            Bounds.Origin.Z - Bounds.BoxExtent.Z;

        const float CapsuleBottomZ =
            -Capsule->GetScaledCapsuleHalfHeight();

        const float OffsetZ =
            CapsuleBottomZ - MeshBottomZ;

        Mesh->AddRelativeLocation(
            FVector(0.0f, 0.0f, OffsetZ)
        );
        
        // Correct mesh forward-facing direction — the skeletal mesh's
        // default orientation is 90 degrees off from the capsule/actor's
        // forward axis, so rotate it to align.
        Mesh->AddRelativeRotation(
            FRotator(0.0f, -90.0f, 0.0f)
        );
    }
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
        // Locomotion (Full Body)
        // --------------------------------------------------------
        
        FAnimationDefinition Idle;
        Idle.Key = EAnimationKey::Idle;
        Idle.PlaybackMode = EAnimationPlaybackMode::Loop;
        Idle.PlayRate = 1.0f;
        Idle.LoopCount = -1;
        Idle.BlendInTime = 0.15f;
        Idle.BlendOutTime = 0.15f;
        Idle.SlotName = "locomotion";
        Idle.Description = FText::FromString(TEXT("Character idle animation."));
        AnimationRegistryComponent->Animations.Add(Idle);
        
        FAnimationDefinition Walk;
        Walk.Key = EAnimationKey::Walk;
        Walk.PlaybackMode = EAnimationPlaybackMode::Loop;
        Walk.PlayRate = 1.0f;
        Walk.LoopCount = -1;
        Walk.BlendInTime = 0.15f;
        Walk.BlendOutTime = 0.15f;
        Walk.SlotName = "locomotion";
        Walk.Description = FText::FromString(TEXT("Character walk animation."));
        AnimationRegistryComponent->Animations.Add(Walk);
        
        FAnimationDefinition Run;
        Run.Key = EAnimationKey::Run;
        Run.PlaybackMode = EAnimationPlaybackMode::Loop;
        Run.PlayRate = 1.0f;
        Run.LoopCount = -1;
        Run.BlendInTime = 0.15f;
        Run.BlendOutTime = 0.15f;
        Run.SlotName = "locomotion";
        Run.Description = FText::FromString(TEXT("Character run animation."));
        AnimationRegistryComponent->Animations.Add(Run);
        
        FAnimationDefinition Sprint;
        Sprint.Key = EAnimationKey::Sprint;
        Sprint.PlaybackMode = EAnimationPlaybackMode::Loop;
        Sprint.PlayRate = 1.0f;
        Sprint.LoopCount = -1;
        Sprint.BlendInTime = 0.15f;
        Sprint.BlendOutTime = 0.15f;
        Sprint.SlotName = "locomotion";
        Sprint.Description = FText::FromString(TEXT("Character sprint animation."));
        AnimationRegistryComponent->Animations.Add(Sprint);
        
        // --------------------------------------------------------
        // Airborne (Full Body)
        // --------------------------------------------------------
        
        FAnimationDefinition Jump;
        Jump.Key = EAnimationKey::Jump;
        Jump.PlaybackMode = EAnimationPlaybackMode::Once;
        Jump.PlayRate = 1.0f;
        Jump.LoopCount = 1;
        Jump.BlendInTime = 0.05f;
        Jump.BlendOutTime = 0.1f;
        Jump.SlotName = "locomotion";
        Jump.Description = FText::FromString(TEXT("Character jump animation."));
        AnimationRegistryComponent->Animations.Add(Jump);
        
        FAnimationDefinition Fall;
        Fall.Key = EAnimationKey::Fall;
        Fall.PlaybackMode = EAnimationPlaybackMode::Loop;
        Fall.PlayRate = 1.0f;
        Fall.LoopCount = -1;
        Fall.BlendInTime = 0.1f;
        Fall.BlendOutTime = 0.1f;
        Fall.SlotName = "locomotion";
        Fall.Description = FText::FromString(TEXT("Character fall animation."));
        AnimationRegistryComponent->Animations.Add(Fall);
        
        FAnimationDefinition Land;
        Land.Key = EAnimationKey::Land;
        Land.PlaybackMode = EAnimationPlaybackMode::Once;
        Land.PlayRate = 1.0f;
        Land.LoopCount = 1;
        Land.BlendInTime = 0.05f;
        Land.BlendOutTime = 0.1f;
        Land.SlotName = "locomotion";
        Land.Description = FText::FromString(TEXT("Character land animation."));
        AnimationRegistryComponent->Animations.Add(Land);
        
        // --------------------------------------------------------
        // Combat (Upper Body)
        // --------------------------------------------------------
        
        
        FAnimationDefinition Hit;
        Hit.Key = EAnimationKey::Hit;
        Hit.PlaybackMode = EAnimationPlaybackMode::Once;
        Hit.PlayRate = 1.0f;
        Hit.LoopCount = 1;
        Hit.BlendInTime = 0.05f;
        Hit.BlendOutTime = 0.1f;
        Hit.SlotName = "combat";
        Hit.Description = FText::FromString(TEXT("Character hit reaction animation."));
        AnimationRegistryComponent->Animations.Add(Hit);
        
        FAnimationDefinition Death;
        Death.Key = EAnimationKey::Death;
        Death.PlaybackMode = EAnimationPlaybackMode::Once;
        Death.PlayRate = 1.0f;
        Death.LoopCount = 1;
        Death.BlendInTime = 0.05f;
        Death.BlendOutTime = 0.2f;
        Death.SlotName = "combat";
        Death.Description = FText::FromString(TEXT("Character death animation."));
        AnimationRegistryComponent->Animations.Add(Death);
        
        // --------------------------------------------------------
        // Interaction (Varies)
        // --------------------------------------------------------
        
        FAnimationDefinition Interact;
        Interact.Key = EAnimationKey::Interact;
        Interact.PlaybackMode = EAnimationPlaybackMode::Once;
        Interact.PlayRate = 1.0f;
        Interact.LoopCount = 1;
        Interact.BlendInTime = 0.1f;
        Interact.BlendOutTime = 0.1f;
        Interact.SlotName = "combat";  // Reaching/interacting with objects
        Interact.Description = FText::FromString(TEXT("Character interact animation."));
        AnimationRegistryComponent->Animations.Add(Interact);
        
        FAnimationDefinition Mount;
        Mount.Key = EAnimationKey::Mount;
        Mount.PlaybackMode = EAnimationPlaybackMode::Once;
        Mount.PlayRate = 1.0f;
        Mount.LoopCount = 1;
        Mount.BlendInTime = 0.1f;
        Mount.BlendOutTime = 0.1f;
        Mount.SlotName = "combat";  // Full body movement to mount
        Mount.Description = FText::FromString(TEXT("Character mount animation."));
        AnimationRegistryComponent->Animations.Add(Mount);
        
        FAnimationDefinition Dismount;
        Dismount.Key = EAnimationKey::Dismount;
        Dismount.PlaybackMode = EAnimationPlaybackMode::Once;
        Dismount.PlayRate = 1.0f;
        Dismount.LoopCount = 1;
        Dismount.BlendInTime = 0.1f;
        Dismount.BlendOutTime = 0.1f;
        Dismount.SlotName = "combat";  // Full body movement to dismount
        Dismount.Description = FText::FromString(TEXT("Character dismount animation."));
        AnimationRegistryComponent->Animations.Add(Dismount);
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
