#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterBrainComponent.generated.h"


UENUM(BlueprintType)
enum class EBrainIntent : uint8
{
    None,
    Idle,
    Wander,
    Follow,
    GoTo,
    Investigate,
    Interact,
    Flee,
    Defend,
    Attack
};


UENUM(BlueprintType)
enum class EBrainFeeling : uint8
{
    Neutral,
    Happy,
    Sad,
    Angry,
    Afraid,
    Curious,
    Suspicious,
    Excited
};


USTRUCT(BlueprintType)
struct FBrainPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Personality",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float Bravery = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Personality",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Personality",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Personality",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float Sociability = 0.5f;
};


USTRUCT(BlueprintType)
struct FBrainIntentData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Intent")
    EBrainIntent Intent = EBrainIntent::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Intent",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float Effort = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Intent")
    EBrainFeeling Feeling = EBrainFeeling::Neutral;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Intent")
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Intent")
    FVector TargetLocation = FVector::ZeroVector;

    void Clear()
    {
        Intent = EBrainIntent::None;
        Effort = 0.0f;
        Feeling = EBrainFeeling::Neutral;
        TargetActor = nullptr;
        TargetLocation = FVector::ZeroVector;
    }
};


UCLASS(ClassGroup=(Gameplay), meta=(BlueprintSpawnableComponent))
class CARIBBEANUNTITLED_API UCharacterBrainComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UCharacterBrainComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Brain")
    FBrainPersonality Personality;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Brain")
    FBrainIntentData CurrentIntent;

public:

    UFUNCTION(BlueprintCallable, Category="Brain")
    void SetIntent(EBrainIntent NewIntent, float Effort = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Brain")
    void SetFeeling(EBrainFeeling NewFeeling);

    UFUNCTION(BlueprintCallable, Category="Brain")
    void SetTargetActor(AActor* Target);

    UFUNCTION(BlueprintCallable, Category="Brain")
    void SetTargetLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category="Brain")
    void ClearIntent();

    const FBrainIntentData& GetIntent() const
    {
        return CurrentIntent;
    }

    EBrainIntent GetCurrentIntent() const
    {
        return CurrentIntent.Intent;
    }

    float GetEffort() const
    {
        return CurrentIntent.Effort;
    }

    EBrainFeeling GetFeeling() const
    {
        return CurrentIntent.Feeling;
    }
    
};
