//
// CharacterActor.cpp
//

#include "CharacterActor.h"

#include "../Components/ControlComponent.h"
#include "../Components/StatsComponent.h"
#include "../Components/AnimationRegistryComponent.h"
#include "../Components/CombatVolumeComponent.h"
#include "../Settings/CaribbeanAssetSettings.h"
#include "../Systems/ActorRegistry.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "Curves/CurveFloat.h"
#include "Net/UnrealNetwork.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


ACharacterActor::ACharacterActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    bReplicates = true;
    SetReplicateMovement(true);
    
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

    ReplicatedCharacterVisualProfileKey = CharacterVisualProfileKey;
    ApplyCharacterVisualProfile();

    
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
    
    HeadCombatVolume =
        CreateDefaultSubobject<UCombatVolumeComponent>(
            TEXT("HeadCombatVolume")
        );

    HeadCombatVolume->OwnerType = ECombatVolumeOwnerType::Body;
    HeadCombatVolume->BodyPart = EBodyPart::Head;
    HeadCombatVolume->ShapeType = ECombatVolumeShape::Sphere;
    HeadCombatVolume->SphereRadius = 20.0f;
    HeadCombatVolume->bStartEnabled = true;
    HeadCombatVolume->bAttackActive = false;


    TorsoCombatVolume =
        CreateDefaultSubobject<UCombatVolumeComponent>(
            TEXT("TorsoCombatVolume")
        );

    TorsoCombatVolume->OwnerType = ECombatVolumeOwnerType::Body;
    TorsoCombatVolume->BodyPart = EBodyPart::Torso;
    TorsoCombatVolume->ShapeType = ECombatVolumeShape::Capsule;
    TorsoCombatVolume->CapsuleRadius = 25.0f;
    TorsoCombatVolume->CapsuleHalfHeight = 45.0f;
    TorsoCombatVolume->bStartEnabled = true;
    TorsoCombatVolume->bAttackActive = false;


    LeftHandCombatVolume =
        CreateDefaultSubobject<UCombatVolumeComponent>(
            TEXT("LeftHandCombatVolume")
        );

    LeftHandCombatVolume->OwnerType = ECombatVolumeOwnerType::Body;
    LeftHandCombatVolume->BodyPart = EBodyPart::LeftHand;
    LeftHandCombatVolume->ShapeType = ECombatVolumeShape::Sphere;
    LeftHandCombatVolume->SphereRadius = 10.0f;
    LeftHandCombatVolume->bStartEnabled = true;
    LeftHandCombatVolume->bAttackActive = false;


    RightHandCombatVolume =
        CreateDefaultSubobject<UCombatVolumeComponent>(
            TEXT("RightHandCombatVolume")
        );

    RightHandCombatVolume->OwnerType = ECombatVolumeOwnerType::Body;
    RightHandCombatVolume->BodyPart = EBodyPart::RightHand;
    RightHandCombatVolume->ShapeType = ECombatVolumeShape::Sphere;
    RightHandCombatVolume->SphereRadius = 10.0f;
    RightHandCombatVolume->bStartEnabled = true;
    RightHandCombatVolume->bAttackActive = false;


    LeftFootCombatVolume =
        CreateDefaultSubobject<UCombatVolumeComponent>(
            TEXT("LeftFootCombatVolume")
        );

    LeftFootCombatVolume->OwnerType = ECombatVolumeOwnerType::Body;
    LeftFootCombatVolume->BodyPart = EBodyPart::LeftFoot;
    LeftFootCombatVolume->ShapeType = ECombatVolumeShape::Sphere;
    LeftFootCombatVolume->SphereRadius = 20.0f;
    LeftFootCombatVolume->bStartEnabled = true;
    LeftFootCombatVolume->bAttackActive = false;


    RightFootCombatVolume =
        CreateDefaultSubobject<UCombatVolumeComponent>(
            TEXT("RightFootCombatVolume")
        );

    RightFootCombatVolume->OwnerType = ECombatVolumeOwnerType::Body;
    RightFootCombatVolume->BodyPart = EBodyPart::RightFoot;
    RightFootCombatVolume->ShapeType = ECombatVolumeShape::Sphere;
    RightFootCombatVolume->SphereRadius = 20.0f;
    RightFootCombatVolume->bStartEnabled = true;
    RightFootCombatVolume->bAttackActive = false;

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
        FAnimationDefinition LightRightPunch;
        LightRightPunch.Key = EAnimationKey::LightRightPunch;
        LightRightPunch.PlaybackMode = EAnimationPlaybackMode::Once;
        LightRightPunch.PlayRate = 1.0f;
        LightRightPunch.LoopCount = 1;
        LightRightPunch.BlendInTime = 0.05f;
        LightRightPunch.BlendOutTime = 0.2f;
        LightRightPunch.SlotName = "combat";
        LightRightPunch.Description = FText::FromString(TEXT("Character light right punch animation."));
        AnimationRegistryComponent->Animations.Add(LightRightPunch);
        
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
        Movement->bUseControllerDesiredRotation = false;
        Movement->RotationRate = FRotator(
            0.0f,
            720.0f,
            0.0f
        );
    }

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

void ACharacterActor::ApplyCharacterVisualProfile()
{
    const UCaribbeanAssetSettings& AssetSettings = UCaribbeanAssetSettings::Get();
    USkeletalMeshComponent* Mesh = GetMesh();
    if (!Mesh)
    {
        return;
    }

    const FCharacterVisualProfile* CharacterProfile =
        AssetSettings.FindCharacterVisualProfile(ReplicatedCharacterVisualProfileKey);
    if (!CharacterProfile)
    {
        return;
    }

    if (USkeletalMesh* SkeletalMesh =
        CharacterProfile->SkeletalMesh.IsValid()
            ? Cast<USkeletalMesh>(CharacterProfile->SkeletalMesh.TryLoad())
            : nullptr)
    {
        Mesh->SetSkeletalMesh(SkeletalMesh);
    }

    if (UClass* AnimBlueprintClass =
        CharacterProfile->AnimBlueprintClass.TryLoadClass<UAnimInstance>())
    {
        Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        Mesh->SetAnimInstanceClass(AnimBlueprintClass);
    }
}

void ACharacterActor::OnRep_CharacterVisualProfileKey()
{
    ApplyCharacterVisualProfile();
}

void ACharacterActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACharacterActor, ReplicatedCharacterVisualProfileKey);
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
    
    USkeletalMeshComponent* Mesh = GetMesh();

    const UCaribbeanAssetSettings& AssetSettings = UCaribbeanAssetSettings::Get();
    const FCombatAttachmentProfile* CombatAttachmentProfile =
        AssetSettings.FindCombatAttachmentProfile(CombatAttachmentProfileKey);
    const FString CombatAttachmentProfileKeyString = CombatAttachmentProfileKey.ToString();

    if (!CombatAttachmentProfile)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Character '%s' has no CombatAttachmentProfile for key '%s'. Using default bone names."
            ),
            *GetName(),
            *CombatAttachmentProfileKeyString
        );
    }

    auto ResolveBoneName = [CombatAttachmentProfile](EBodyPart BodyPart) -> TOptional<FName>
    {
        if (CombatAttachmentProfile)
        {
            if (const FName* BoneName = CombatAttachmentProfile->BoneNamesByBodyPart.Find(BodyPart))
            {
                return *BoneName;
            }
        }

        return TOptional<FName>();
    };

    auto AttachCombatVolume = [Mesh, ResolveBoneName, CombatAttachmentProfileKeyString](UCombatVolumeComponent* CombatVolume, EBodyPart BodyPart)
    {
        if (!CombatVolume)
        {
            return;
        }

        const TOptional<FName> BoneNameOpt = ResolveBoneName(BodyPart);

        if (!BoneNameOpt.IsSet())
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "No combat attachment bone is defined for body part '%s' in profile '%s'."
                ),
                *StaticEnum<EBodyPart>()->GetValueAsString(BodyPart),
                *CombatAttachmentProfileKeyString
            );
            return;
        }

        const FName BoneName = BoneNameOpt.GetValue();

        if (!Mesh)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("Failed to attach %s: mesh is null."),
                *GetNameSafe(CombatVolume)
            );
            return;
        }

        if (!Mesh->DoesSocketExist(BoneName) &&
            Mesh->GetBoneIndex(BoneName) == INDEX_NONE)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "Attaching %s to '%s', but the mesh has no matching socket or bone."
                ),
                *GetNameSafe(CombatVolume),
                *BoneName.ToString()
            );
        }

        const bool bAttached =
            CombatVolume->AttachToComponent(
                Mesh,
                BoneName
            );

        if (!bAttached)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "Failed to attach %s to '%s'."
                ),
                *GetNameSafe(CombatVolume),
                *BoneName.ToString()
            );
            return;
        }

        UShapeComponent* CollisionShape = CombatVolume->GetCollisionShape();
        if (!IsValid(CollisionShape))
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "%s attached to '%s', but CollisionShape is invalid."
                ),
                *GetNameSafe(CombatVolume),
                *BoneName.ToString()
            );
            return;
        }

        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "Attached %s to '%s' | Parent: %s | WorldLocation: %s | WorldRotation: %s"
            ),
            *GetNameSafe(CombatVolume),
            *BoneName.ToString(),
            *GetNameSafe(CollisionShape->GetAttachParent()),
            *CollisionShape->GetComponentLocation().ToString(),
            *CollisionShape->GetComponentRotation().ToString()
        );
    };

    AttachCombatVolume(HeadCombatVolume, EBodyPart::Head);
    AttachCombatVolume(TorsoCombatVolume, EBodyPart::Torso);
    AttachCombatVolume(LeftHandCombatVolume, EBodyPart::LeftHand);
    AttachCombatVolume(RightHandCombatVolume, EBodyPart::RightHand);
    AttachCombatVolume(LeftFootCombatVolume, EBodyPart::LeftFoot);
    AttachCombatVolume(RightFootCombatVolume, EBodyPart::RightFoot);

    if (UWorld* World = GetWorld())
    {
        if (UActorRegistry* Registry =
            World->GetSubsystem<UActorRegistry>())
        {
            Registry->RegisterActor(this);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to register %s: ActorRegistry subsystem not found."), *GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to register %s: world is null."), *GetName());
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
