//
// IKSystem.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tickable.h"
#include "../Events/IKEventPayload.h"
#include "IKSystem.generated.h"

class UActorRegistry;
class UEventBusSubsystem;
class UIKComponent;

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

private:
    struct FIKState
    {
        float TimeUntilNextUpdate = 0.0f;
    };

    TMap<TWeakObjectPtr<AActor>, FIKState> States;
    bool bSubscribed = false;

    UEventBusSubsystem* GetEventBus() const;
    UActorRegistry* GetActorRegistry() const;
    void OnIKEvent(const FIKEventPayload& Payload);
};
