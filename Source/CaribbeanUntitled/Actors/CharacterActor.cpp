//
// CharacterActor.cpp
//

#include "CharacterActor.h"

#include "../Components/ControlComponent.h"
#include "../Components/StatsComponent.h"
#include "../Components/AnimationRegistryComponent.h"
#include "../Components/CombatComponent.h"
#include "../Components/AttackStateComponent.h"
#include "../Components/CombatVolumeComponent.h"
#include "../Components/MoveSetComponent.h"
#include "../Settings/CaribbeanAssetSettings.h"
#include "../Systems/ActorRegistry.h"
#include "../Systems/AnimationSystem.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "Curves/CurveFloat.h"
#include "Net/UnrealNetwork.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


ACharacterActor::ACharacterActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    bReplicates = true;
    SetReplicateMovement(true);
    
    USkeletalMeshComponent* Mesh = GetMesh();
    UCapsuleComponent* Capsule = GetCapsuleComponent();

    if (Mesh && Capsule)
    {
        Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
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

    PoseableMeshComponent =
        CreateDefaultSubobject<UPoseableMeshComponent>(
            TEXT("PoseableMeshComponent")
        );

    if (PoseableMeshComponent && Mesh)
    {
        PoseableMeshComponent->PrimaryComponentTick.bCanEverTick = true;
        PoseableMeshComponent->PrimaryComponentTick.bStartWithTickEnabled = true;
        PoseableMeshComponent->SetupAttachment(GetCapsuleComponent());
        PoseableMeshComponent->SetRelativeLocation(Mesh->GetRelativeLocation());
        PoseableMeshComponent->SetRelativeRotation(Mesh->GetRelativeRotation());
        PoseableMeshComponent->SetRelativeScale3D(Mesh->GetRelativeScale3D());
        PoseableMeshComponent->SetVisibility(true, true);
        PoseableMeshComponent->SetHiddenInGame(false, true);
        PoseableMeshComponent->AddTickPrerequisiteComponent(Mesh);
        Mesh->SetVisibility(false, true);
        Mesh->SetHiddenInGame(true, true);
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

    MoveSetComponent =
        CreateDefaultSubobject<UMoveSetComponent>(
            TEXT("MoveSetComponent")
        );

    CombatComponent =
        CreateDefaultSubobject<UCombatComponent>(
            TEXT("CombatComponent")
        );

    AttackStateComponent =
        CreateDefaultSubobject<UAttackStateComponent>(
            TEXT("AttackStateComponent")
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
        Idle.BlendInTime = 0.2f;
        Idle.BlendOutTime = 0.2f;
        Idle.Layer = EAnimationLayer::FullBody;
        Idle.Description = FText::FromString(TEXT("Character idle animation."));
        AnimationRegistryComponent->Animations.Add(Idle);
        
        FAnimationDefinition Walk;
        Walk.Key = EAnimationKey::Walk;
        Walk.PlaybackMode = EAnimationPlaybackMode::Loop;
        Walk.PlayRate = 1.0f;
        Walk.BlendInTime = 0.2f;
        Walk.BlendOutTime = 0.2f;
        Walk.Layer = EAnimationLayer::FullBody;
        Walk.Description = FText::FromString(TEXT("Character walk animation."));
        AnimationRegistryComponent->Animations.Add(Walk);

        FSoftObjectPath WalkBackwardsPath(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Walk_Backwards.Walk_Backwards")
        );

        FAnimationDefinition WalkBackwards;
        WalkBackwards.Key = EAnimationKey::WalkBackwards;
        WalkBackwards.PlaybackMode = EAnimationPlaybackMode::Loop;
        WalkBackwards.PlayRate = 1.0f;
        WalkBackwards.BlendInTime = 0.2f;
        WalkBackwards.BlendOutTime = 0.2f;
        WalkBackwards.Layer = EAnimationLayer::FullBody;
        WalkBackwards.Animation = Cast<UAnimSequence>(WalkBackwardsPath.TryLoad());
        WalkBackwards.Description = FText::FromString(TEXT("Character walk backwards animation."));
        AnimationRegistryComponent->Animations.Add(WalkBackwards);

        FSoftObjectPath LeftStrafePath(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Left_Strafe.Left_Strafe")
        );

        FAnimationDefinition LeftStrafe;
        LeftStrafe.Key = EAnimationKey::LeftStrafe;
        LeftStrafe.PlaybackMode = EAnimationPlaybackMode::Loop;
        LeftStrafe.PlayRate = 1.0f;
        LeftStrafe.BlendInTime = 0.2f;
        LeftStrafe.BlendOutTime = 0.2f;
        LeftStrafe.Layer = EAnimationLayer::FullBody;
        LeftStrafe.Animation = Cast<UAnimSequence>(LeftStrafePath.TryLoad());
        LeftStrafe.Description = FText::FromString(TEXT("Character left strafe animation."));
        AnimationRegistryComponent->Animations.Add(LeftStrafe);

        FSoftObjectPath RightStrafePath(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Right_Strafe.Right_Strafe")
        );

        FAnimationDefinition RightStrafe;
        RightStrafe.Key = EAnimationKey::RightStrafe;
        RightStrafe.PlaybackMode = EAnimationPlaybackMode::Loop;
        RightStrafe.PlayRate = 1.0f;
        RightStrafe.BlendInTime = 0.2f;
        RightStrafe.BlendOutTime = 0.2f;
        RightStrafe.Layer = EAnimationLayer::FullBody;
        RightStrafe.Animation = Cast<UAnimSequence>(RightStrafePath.TryLoad());
        RightStrafe.Description = FText::FromString(TEXT("Character right strafe animation."));
        AnimationRegistryComponent->Animations.Add(RightStrafe);

        FSoftObjectPath LeftStrafeRunPath(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Left_Strafe_Run.Left_Strafe_Run")
        );

        FAnimationDefinition LeftStrafeRun;
        LeftStrafeRun.Key = EAnimationKey::LeftStrafeRun;
        LeftStrafeRun.PlaybackMode = EAnimationPlaybackMode::Loop;
        LeftStrafeRun.PlayRate = 1.0f;
        LeftStrafeRun.BlendInTime = 0.18f;
        LeftStrafeRun.BlendOutTime = 0.18f;
        LeftStrafeRun.Layer = EAnimationLayer::FullBody;
        LeftStrafeRun.Animation = Cast<UAnimSequence>(LeftStrafeRunPath.TryLoad());
        LeftStrafeRun.Description = FText::FromString(TEXT("Character left strafe run animation."));
        AnimationRegistryComponent->Animations.Add(LeftStrafeRun);

        FSoftObjectPath RightStrafeRunPath(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Right_Strafe_Run.Right_Strafe_Run")
        );

        FAnimationDefinition RightStrafeRun;
        RightStrafeRun.Key = EAnimationKey::RightStrafeRun;
        RightStrafeRun.PlaybackMode = EAnimationPlaybackMode::Loop;
        RightStrafeRun.PlayRate = 1.0f;
        RightStrafeRun.BlendInTime = 0.18f;
        RightStrafeRun.BlendOutTime = 0.18f;
        RightStrafeRun.Layer = EAnimationLayer::FullBody;
        RightStrafeRun.Animation = Cast<UAnimSequence>(RightStrafeRunPath.TryLoad());
        RightStrafeRun.Description = FText::FromString(TEXT("Character right strafe run animation."));
        AnimationRegistryComponent->Animations.Add(RightStrafeRun);
        
        FAnimationDefinition Run;
        Run.Key = EAnimationKey::Run;
        Run.PlaybackMode = EAnimationPlaybackMode::Loop;
        Run.PlayRate = 1.0f;
        Run.BlendInTime = 0.18f;
        Run.BlendOutTime = 0.18f;
        Run.Layer = EAnimationLayer::FullBody;
        Run.Description = FText::FromString(TEXT("Character run animation."));
        AnimationRegistryComponent->Animations.Add(Run);
        
        FAnimationDefinition Sprint;
        Sprint.Key = EAnimationKey::Sprint;
        Sprint.PlaybackMode = EAnimationPlaybackMode::Loop;
        Sprint.PlayRate = 1.0f;
        Sprint.BlendInTime = 0.18f;
        Sprint.BlendOutTime = 0.18f;
        Sprint.Layer = EAnimationLayer::FullBody;
        Sprint.Description = FText::FromString(TEXT("Character sprint animation."));
        AnimationRegistryComponent->Animations.Add(Sprint);
        
        // --------------------------------------------------------
        // Airborne (Full Body)
        // --------------------------------------------------------
        
        FAnimationDefinition Jump;
        Jump.Key = EAnimationKey::Jump;
        Jump.PlaybackMode = EAnimationPlaybackMode::Once;
        Jump.PlayRate = 1.0f;
        Jump.BlendInTime = 0.08f;
        Jump.BlendOutTime = 0.12f;
        Jump.Layer = EAnimationLayer::FullBody;
        Jump.Description = FText::FromString(TEXT("Character jump animation."));
        AnimationRegistryComponent->Animations.Add(Jump);
        
        FAnimationDefinition Fall;
        Fall.Key = EAnimationKey::Fall;
        Fall.PlaybackMode = EAnimationPlaybackMode::Loop;
        Fall.PlayRate = 1.0f;
        Fall.BlendInTime = 0.12f;
        Fall.BlendOutTime = 0.12f;
        Fall.Layer = EAnimationLayer::FullBody;
        Fall.Description = FText::FromString(TEXT("Character fall animation."));
        AnimationRegistryComponent->Animations.Add(Fall);
        
        FAnimationDefinition Land;
        Land.Key = EAnimationKey::Land;
        Land.PlaybackMode = EAnimationPlaybackMode::Once;
        Land.PlayRate = 1.0f;
        Land.BlendInTime = 0.08f;
        Land.BlendOutTime = 0.12f;
        Land.Layer = EAnimationLayer::FullBody;
        Land.Description = FText::FromString(TEXT("Character land animation."));
        AnimationRegistryComponent->Animations.Add(Land);
        
        // --------------------------------------------------------
        // Combat (Upper Body)
        // --------------------------------------------------------
        FAnimationDefinition LightRightPunch;
        LightRightPunch.Key = EAnimationKey::LightRightPunch;
        LightRightPunch.PlaybackMode = EAnimationPlaybackMode::Once;
        LightRightPunch.PlayRate = 1.0f;
        LightRightPunch.BlendInTime = 0.1f;
        LightRightPunch.BlendOutTime = 0.18f;
        LightRightPunch.Layer = EAnimationLayer::UpperBody;
        LightRightPunch.Breakpoint = EAnimationBreakpoint::SpineAndDown;
        LightRightPunch.Description = FText::FromString(TEXT("Character light right punch animation."));
        AnimationRegistryComponent->Animations.Add(LightRightPunch);

        FSoftObjectPath LightLeftPunchPath(
            TEXT("/Game/Assets/Characters/Demo/Demo-Bot/Animations/Light_Left_Punch.Light_Left_Punch")
        );

        FAnimationDefinition LightLeftPunch;
        LightLeftPunch.Key = EAnimationKey::LightLeftPunch;
        LightLeftPunch.PlaybackMode = EAnimationPlaybackMode::Once;
        LightLeftPunch.PlayRate = 1.0f;
        LightLeftPunch.BlendInTime = 0.1f;
        LightLeftPunch.BlendOutTime = 0.18f;
        LightLeftPunch.Layer = EAnimationLayer::UpperBody;
        LightLeftPunch.Breakpoint = EAnimationBreakpoint::SpineAndDown;
        LightLeftPunch.Animation = Cast<UAnimSequence>(LightLeftPunchPath.TryLoad());
        LightLeftPunch.Description = FText::FromString(TEXT("Character light left punch animation."));
        AnimationRegistryComponent->Animations.Add(LightLeftPunch);
        
        FAnimationDefinition Hit;
        Hit.Key = EAnimationKey::Hit;
        Hit.PlaybackMode = EAnimationPlaybackMode::Once;
        Hit.PlayRate = 1.0f;
        Hit.BlendInTime = 0.08f;
        Hit.BlendOutTime = 0.14f;
        Hit.Layer = EAnimationLayer::UpperBody;
        Hit.Breakpoint = EAnimationBreakpoint::SpineAndDown;
        Hit.Description = FText::FromString(TEXT("Character hit reaction animation."));
        AnimationRegistryComponent->Animations.Add(Hit);
        
        FAnimationDefinition Death;
        Death.Key = EAnimationKey::Death;
        Death.PlaybackMode = EAnimationPlaybackMode::Once;
        Death.PlayRate = 1.0f;
        Death.BlendInTime = 0.1f;
        Death.BlendOutTime = 0.2f;
        Death.Layer = EAnimationLayer::FullBody;
        Death.Breakpoint = EAnimationBreakpoint::SpineAndDown;
        Death.Description = FText::FromString(TEXT("Character death animation."));
        AnimationRegistryComponent->Animations.Add(Death);
        
        // --------------------------------------------------------
        // Interaction (Varies)
        // --------------------------------------------------------
        
        FAnimationDefinition Interact;
        Interact.Key = EAnimationKey::Interact;
        Interact.PlaybackMode = EAnimationPlaybackMode::Once;
        Interact.PlayRate = 1.0f;
        Interact.BlendInTime = 0.12f;
        Interact.BlendOutTime = 0.12f;
        Interact.Layer = EAnimationLayer::UpperBody;
        Interact.Breakpoint = EAnimationBreakpoint::SpineAndDown;
        Interact.Description = FText::FromString(TEXT("Character interact animation."));
        AnimationRegistryComponent->Animations.Add(Interact);
        
        FAnimationDefinition Mount;
        Mount.Key = EAnimationKey::Mount;
        Mount.PlaybackMode = EAnimationPlaybackMode::Once;
        Mount.PlayRate = 1.0f;
        Mount.BlendInTime = 0.12f;
        Mount.BlendOutTime = 0.16f;
        Mount.Layer = EAnimationLayer::FullBody;
        Mount.Breakpoint = EAnimationBreakpoint::FullBody;
        Mount.Description = FText::FromString(TEXT("Character mount animation."));
        AnimationRegistryComponent->Animations.Add(Mount);
        
        FAnimationDefinition Dismount;
        Dismount.Key = EAnimationKey::Dismount;
        Dismount.PlaybackMode = EAnimationPlaybackMode::Once;
        Dismount.PlayRate = 1.0f;
        Dismount.BlendInTime = 0.12f;
        Dismount.BlendOutTime = 0.16f;
        Dismount.Layer = EAnimationLayer::FullBody;
        Dismount.Breakpoint = EAnimationBreakpoint::FullBody;
        Dismount.Description = FText::FromString(TEXT("Character dismount animation."));
        AnimationRegistryComponent->Animations.Add(Dismount);
    }

    if (MoveSetComponent)
    {
        FGameplayCombatMove LightAttack;
        LightAttack.MoveId = TEXT("LightAttack1");
        LightAttack.Animation = EAnimationKey::LightLeftPunch;
        LightAttack.AnticipationTime = 0.10f;
        LightAttack.ActiveTime = 0.55f;
        LightAttack.RecoveryTime = 0.15f;
        LightAttack.Damage = 10.0f;
        LightAttack.Stamina = 10.0f;
        LightAttack.CombatVolumes.Add(LeftHandCombatVolume);

        MoveSetComponent->AddMove(
            EMoveSetTypeKey::LightAttack1,
            LightAttack
        );
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
        Movement->bOrientRotationToMovement = false;
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

void ACharacterActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (DeltaSeconds <= 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const ENetMode NetMode = World->GetNetMode();
    if (NetMode == NM_DedicatedServer)
    {
        return;
    }

    UpdatePoseableMeshFromCurrentAnimation(DeltaSeconds);
}

void ACharacterActor::ApplyCharacterVisualProfile()
{
    const UCaribbeanAssetSettings& AssetSettings = UCaribbeanAssetSettings::Get();
    USkeletalMeshComponent* Mesh = GetMesh();
    UPoseableMeshComponent* PoseableMesh = PoseableMeshComponent.Get();
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
        if (PoseableMesh)
        {
            PoseableMesh->SetSkeletalMesh(SkeletalMesh);
        }
    }

    if (UClass* AnimBlueprintClass =
        CharacterProfile->AnimBlueprintClass.TryLoadClass<UAnimInstance>())
    {
        Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        Mesh->SetAnimInstanceClass(AnimBlueprintClass);
    }
}

void ACharacterActor::SyncPoseableMeshFromSkeletalMesh()
{
    USkeletalMeshComponent* Mesh = GetMesh();
    UPoseableMeshComponent* PoseableMesh = PoseableMeshComponent.Get();
    if (!Mesh || !PoseableMesh)
    {
        return;
    }

    Mesh->RefreshBoneTransforms();
    PoseableMesh->CopyPoseFromSkeletalComponent(Mesh);
    PoseableMesh->RefreshBoneTransforms();
    PoseableMesh->MarkRefreshTransformDirty();
}

void ACharacterActor::UpdatePoseableMeshFromCurrentAnimation(float DeltaSeconds)
{
    if (DeltaSeconds <= 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    UPoseableMeshComponent* PoseableMesh = PoseableMeshComponent.Get();
    if (!PoseableMesh)
    {
        return;
    }

    UAnimationSystem* AnimationSystem = World->GetSubsystem<UAnimationSystem>();
    if (!AnimationSystem)
    {
        return;
    }

    USkeletalMeshComponent* ReferenceMesh = GetMesh();
    if (!ReferenceMesh || !ReferenceMesh->SkeletalMesh)
    {
        return;
    }

    const USkeleton* Skeleton = ReferenceMesh->SkeletalMesh->GetSkeleton();
    if (!Skeleton)
    {
        return;
    }

    const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
    const int32 BoneCount = RefSkeleton.GetNum();
    TArray<FTransform> ComponentSpaceTransforms;
    ComponentSpaceTransforms.SetNum(BoneCount);

    struct FLayerSample
    {
        const FAnimationDefinition* Definition = nullptr;
        const UAnimSequence* Sequence = nullptr;
        float SampleTime = 0.0f;
        float BlendAlpha = 1.0f;
        EAnimationBreakpoint Breakpoint = EAnimationBreakpoint::FullBody;
    };

    const auto SampleLayer = [AnimationSystem, this](EAnimationLayer Layer) -> FLayerSample
    {
        FLayerSample Sample;
        Sample.Definition = AnimationSystem->GetCurrentAnimationDefinition(this, Layer);
        Sample.Sequence = Sample.Definition
            ? Cast<UAnimSequence>(Sample.Definition->Animation)
            : nullptr;

        if (!Sample.Definition || !Sample.Sequence)
        {
            return Sample;
        }

        const float Elapsed = AnimationSystem->GetStateElapsedTime(this, Layer);
        const float PlayRate = Sample.Definition->PlayRate;
        const float Length = Sample.Sequence->GetPlayLength();
        Sample.SampleTime = Length > 0.0f ? FMath::Fmod(Elapsed * PlayRate, Length) : 0.0f;
        Sample.Breakpoint = Sample.Definition->Breakpoint;

        if (Layer == EAnimationLayer::UpperBody)
        {
            const float BlendIn = FMath::Max(0.0f, Sample.Definition->BlendInTime);
            const float BlendOut = FMath::Max(0.0f, Sample.Definition->BlendOutTime);
            const float BlendInAlpha = BlendIn > 0.0f ? FMath::Clamp(Elapsed / BlendIn, 0.0f, 1.0f) : 1.0f;
            const float BlendOutAlpha =
                (BlendOut > 0.0f && Length > 0.0f)
                    ? FMath::Clamp((Length - Elapsed) / BlendOut, 0.0f, 1.0f)
                    : 1.0f;
            Sample.BlendAlpha = FMath::Min(BlendInAlpha, BlendOutAlpha);
        }

        return Sample;
    };

    const FLayerSample BaseSample = SampleLayer(EAnimationLayer::FullBody);
    const FLayerSample UpperSample = SampleLayer(EAnimationLayer::UpperBody);

    const int32 UpperBreakpointBoneIndex =
        UpperSample.Definition &&
        UpperSample.Breakpoint != EAnimationBreakpoint::FullBody &&
        !UpperBodyBreakpointBoneName.IsNone()
            ? RefSkeleton.FindBoneIndex(UpperBodyBreakpointBoneName)
            : INDEX_NONE;

    if (UpperSample.Definition &&
        UpperSample.Breakpoint != EAnimationBreakpoint::FullBody &&
        UpperBreakpointBoneIndex == INDEX_NONE)
    {
    }

    const auto IsDescendantOfBone = [&RefSkeleton](int32 BoneIndex, int32 AncestorIndex) -> bool
    {
        if (BoneIndex == INDEX_NONE || AncestorIndex == INDEX_NONE)
        {
            return false;
        }

        int32 CurrentIndex = BoneIndex;
        while (CurrentIndex != INDEX_NONE)
        {
            if (CurrentIndex == AncestorIndex)
            {
                return true;
            }

            CurrentIndex = RefSkeleton.GetParentIndex(CurrentIndex);
        }

        return false;
    };

    for (int32 BoneIndex = 0; BoneIndex < BoneCount; ++BoneIndex)
    {
        const FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
        FTransform BoneLocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];

        if (BaseSample.Sequence)
        {
            BaseSample.Sequence->GetBoneTransform(
                BoneLocalTransform,
                FSkeletonPoseBoneIndex(BoneIndex),
                BaseSample.SampleTime,
                false
            );
        }

        if (UpperSample.Sequence && IsDescendantOfBone(BoneIndex, UpperBreakpointBoneIndex))
        {
            FTransform UpperBoneLocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];
            UpperSample.Sequence->GetBoneTransform(
                UpperBoneLocalTransform,
                FSkeletonPoseBoneIndex(BoneIndex),
                UpperSample.SampleTime,
                false
            );
            BoneLocalTransform.Blend(BoneLocalTransform, UpperBoneLocalTransform, UpperSample.BlendAlpha);
        }

        const int32 ParentIndex = RefSkeleton.GetParentIndex(BoneIndex);
        const FTransform BoneComponentTransform =
            ParentIndex != INDEX_NONE
                ? BoneLocalTransform * ComponentSpaceTransforms[ParentIndex]
                : BoneLocalTransform;

        ComponentSpaceTransforms[BoneIndex] = BoneComponentTransform;
        PoseableMesh->SetBoneTransformByName(BoneName, BoneComponentTransform, EBoneSpaces::ComponentSpace);
    }

    PoseableMesh->RefreshBoneTransforms();
    PoseableMesh->MarkRefreshTransformDirty();
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
            return;
        }

        const FName BoneName = BoneNameOpt.GetValue();

        if (!Mesh)
        {
            return;
        }

        if (!Mesh->DoesSocketExist(BoneName) &&
            Mesh->GetBoneIndex(BoneName) == INDEX_NONE)
        {
        }

        const bool bAttached =
            CombatVolume->AttachToComponent(
                Mesh,
                BoneName
            );

        if (!bAttached)
        {
            return;
        }

        UShapeComponent* CollisionShape = CombatVolume->GetCollisionShape();
        if (!IsValid(CollisionShape))
        {
            return;
        }

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
        }
    }
    else
    {
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
