//
// IKSystem.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tickable.h"
#include "../Components/IKComponent.h"
#include "IKSystem.generated.h"

class UActorRegistry;
class UIKComponent;
class UPoseableMeshComponent;

UCLASS()
class CARIBBEANUNTITLED_API UIKSystem : public UObject
{
    GENERATED_BODY()

public:
    void Cleanup();
    virtual void Tick(float DeltaTime);
    TStatId GetStatId() const;
    bool IsTickable() const { return true; }

    UPROPERTY(EditAnywhere, Category = "IK")
    float UpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, Category = "IK", meta = (ClampMin = "0.0"))
    float BlendSpeed = 4.0f;

    UPROPERTY(EditAnywhere, Category = "IK", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float MaxYaw = 70.0f;

    UPROPERTY(EditAnywhere, Category = "IK", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float MaxPitch = 35.0f;

private:
    struct FIKState
    {
        float TimeUntilNextUpdate = 0.0f;
    };

    TMap<TWeakObjectPtr<AActor>, FIKState> States;

    UActorRegistry* GetActorRegistry() const;
    void ApplyPoseableIK(AActor* Actor, UIKComponent* IK, float DeltaTime);
    void RotateChain(UPoseableMeshComponent* PoseableMesh, const FIKTargetData& Target, float DeltaTime);
};
