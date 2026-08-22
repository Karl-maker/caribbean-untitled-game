//
// IKSystem.cpp
// CaribbeanUntitled
//

#include "IKSystem.h"

#include "ActorRegistry.h"
#include "../Components/IKComponent.h"

#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

TStatId UIKSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UIKSystem, STATGROUP_Tickables);
}

void UIKSystem::Cleanup()
{
    States.Reset();
}

UActorRegistry* UIKSystem::GetActorRegistry() const
{
    UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UActorRegistry>() : nullptr;
}

void UIKSystem::RotateChain(UPoseableMeshComponent* PoseableMesh, const FIKTargetData& Target, float DeltaTime)
{
    if (!PoseableMesh || !Target.bEnabled)
    {
        return;
    }

    const FVector TargetDirection = Target.TargetDirection.GetSafeNormal();
    if (TargetDirection.IsNearlyZero())
    {
        return;
    }

    const FName HeadBone = Target.BoneName.IsNone() ? TEXT("Head") : Target.BoneName;
    const FName ParentBone = Target.ParentBoneName.IsNone() ? TEXT("Neck") : Target.ParentBoneName;

    auto ApplyRotation = [this, PoseableMesh, DeltaTime](const FName BoneName, const FVector& Direction, float Weight)
    {
        const int32 BoneIndex = PoseableMesh->GetBoneIndex(BoneName);
        if (BoneIndex == INDEX_NONE)
        {
            return;
        }

        const FRotator DesiredRot = Direction.Rotation();
        const FTransform Current = PoseableMesh->GetBoneTransformByName(BoneName, EBoneSpaces::ComponentSpace);
        const FRotator CurrentRot = Current.GetRotation().Rotator();

        const float Alpha = FMath::Clamp(DeltaTime * FMath::Max(1.0f, BlendSpeed * Weight), 0.0f, 1.0f);
        const FRotator Smoothed = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, BlendSpeed * Weight);
        FTransform NewTransform = Current;
        NewTransform.SetRotation(Smoothed.Quaternion());
        PoseableMesh->SetBoneTransformByName(BoneName, NewTransform, EBoneSpaces::ComponentSpace);
    };

    const FRotator BaseRot = TargetDirection.Rotation();
    const float ClampedYaw = FMath::Clamp(BaseRot.Yaw, -MaxYaw, MaxYaw);
    const float ClampedPitch = FMath::Clamp(BaseRot.Pitch, -MaxPitch, MaxPitch);
    const FRotator ClampedRot(ClampedPitch, ClampedYaw, 0.0f);

    ApplyRotation(ParentBone, ClampedRot.Vector(), 0.5f);
    ApplyRotation(HeadBone, ClampedRot.Vector(), 1.0f);

    PoseableMesh->MarkRefreshTransformDirty();
}

void UIKSystem::ApplyPoseableIK(AActor* Actor, UIKComponent* IK, float DeltaTime)
{
    if (!Actor || !IK)
    {
        return;
    }

    USkeletalMeshComponent* SourceMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    UPoseableMeshComponent* PoseableMesh = Actor->FindComponentByClass<UPoseableMeshComponent>();
    if (!PoseableMesh || !SourceMesh)
    {
        return;
    }

    PoseableMesh->CopyPoseFromSkeletalComponent(SourceMesh);

    PoseableMesh->CopyPoseFromSkeletalComponent(SourceMesh);

    for (const FIKTargetEntry& Entry : IK->Targets)
    {
        if (!Entry.Target.bEnabled)
        {
            continue;
        }

        RotateChain(PoseableMesh, Entry.Target, DeltaTime);
    }
}

void UIKSystem::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    UActorRegistry* Registry = GetActorRegistry();
    if (!Registry)
    {
        return;
    }

    const TSet<TWeakObjectPtr<AActor>>* Actors = Registry->GetActorsWithComponent<UIKComponent>();
    if (!Actors)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakActor : *Actors)
    {
        AActor* Actor = WeakActor.Get();
        if (!IsValid(Actor))
        {
            continue;
        }

        UIKComponent* IK = Actor->FindComponentByClass<UIKComponent>();
        if (!IK)
        {
            continue;
        }

        FIKState& State = States.FindOrAdd(Actor);
        State.TimeUntilNextUpdate -= DeltaTime;
        if (State.TimeUntilNextUpdate > 0.0f)
        {
            continue;
        }

        State.TimeUntilNextUpdate = UpdateInterval;
        ApplyPoseableIK(Actor, IK, DeltaTime);
    }
}
