//
// AnimationComponent.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimTypes.h"
#include "AnimationComponent.generated.h"


/**
 * Standardized animation keys used by gameplay systems.
 *
 * Gameplay systems should reference these keys rather than knowing
 * which animation assets a particular character uses.
 */
UENUM(BlueprintType)
enum class EAnimationKey : uint8
{
    None        UMETA(DisplayName = "None"),

    // Locomotion
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),

    // Airborne
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),

    // Combat
    Attack_1    UMETA(DisplayName = "Attack 1"),
    Attack_2    UMETA(DisplayName = "Attack 2"),
    Attack_3    UMETA(DisplayName = "Attack 3"),

    Hit         UMETA(DisplayName = "Hit"),
    Death       UMETA(DisplayName = "Death"),

    // Interaction
    Interact    UMETA(DisplayName = "Interact"),
    Mount       UMETA(DisplayName = "Mount"),
    Dismount    UMETA(DisplayName = "Dismount")
};


/**
 * How the animation should behave when played.
 */
UENUM(BlueprintType)
enum class EAnimationPlaybackMode : uint8
{
    Loop        UMETA(DisplayName = "Loop"),
    Once        UMETA(DisplayName = "Play Once"),
    Hold        UMETA(DisplayName = "Hold Last Frame")
};


/**
 * Defines how an animation blends into the current pose.
 */
USTRUCT(BlueprintType)
struct FAnimationBlendSettings
{
    GENERATED_BODY()

    /**
     * Time required to blend from the current animation into this one.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Blend",
        meta = (ClampMin = "0.0")
    )
    float BlendInTime = 0.15f;

    /**
     * Time required to blend out of this animation.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Blend",
        meta = (ClampMin = "0.0")
    )
    float BlendOutTime = 0.15f;

    /**
     * Blend curve used during the transition.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Blend"
    )
    TObjectPtr<UCurveFloat> BlendCurve = nullptr;
};


/**
 * Defines timing and playback behavior for an animation.
 */
USTRUCT(BlueprintType)
struct FAnimationTimingSettings
{
    GENERATED_BODY()

    /**
     * Playback mode for this animation.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Timing"
    )
    EAnimationPlaybackMode PlaybackMode = EAnimationPlaybackMode::Loop;

    /**
     * Playback speed multiplier.
     *
     * 1.0 = normal speed.
     * 2.0 = twice as fast.
     * 0.5 = half speed.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Timing",
        meta = (ClampMin = "0.0")
    )
    float PlayRate = 1.0f;

    /**
     * Optional starting position within the animation.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Timing",
        meta = (ClampMin = "0.0")
    )
    float StartPosition = 0.0f;

    /**
     * Number of times this animation should repeat.
     *
     * -1 = infinite.
     *  1 = play once.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Timing",
        meta = (ClampMin = "-1")
    )
    int32 LoopCount = -1;
};


/**
 * Describes a single animation entry.
 *
 * This struct does NOT play the animation.
 * It only describes how the animation should be used.
 */
USTRUCT(BlueprintType)
struct FAnimationDefinition
{
    GENERATED_BODY()

    /**
     * Gameplay key used to identify this animation.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation"
    )
    EAnimationKey Key = EAnimationKey::None;

    /**
     * Animation asset associated with this key.
     *
     * Can reference Animation Sequence, Montage, or another
     * supported Unreal Animation Asset.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation"
    )
    TObjectPtr<UAnimationAsset> Animation = nullptr;

    /**
     * How this animation blends with the current pose.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation"
    )
    FAnimationBlendSettings Blend;

    /**
     * Timing and playback configuration.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation"
    )
    FAnimationTimingSettings Timing;

    /**
     * Optional gameplay description.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation"
    )
    FText Description;
};


/**
 * Stores and exposes animation state for an Actor.
 *
 * Gameplay systems interact with this component rather than
 * directly manipulating the SkeletalMeshComponent.
 *
 * Example:
 *
 * MovementSystem detects running
 *      ↓
 * AnimationSystem requests EAnimationKey::Run
 *      ↓
 * AnimationComponent resolves Run
 *      ↓
 * AnimationSystem passes the definition to Unreal's animation system
 */
UCLASS(
    ClassGroup=(Gameplay),
    BlueprintType,
    Blueprintable,
    meta=(BlueprintSpawnableComponent)
)
class CARIBBEANUNTITLED_API UAnimationComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UAnimationComponent();


    //=========================================================================
    // Animation Definitions
    //=========================================================================

    /**
     * All animations available to this Actor.
     *
     * Different characters can use the same EAnimationKey while
     * referencing completely different animation assets.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation"
    )
    TArray<FAnimationDefinition> Animations;


    //=========================================================================
    // Current State
    //=========================================================================

    /**
     * Animation currently requested by gameplay.
     */
    UPROPERTY(
        BlueprintReadOnly,
        Category = "Animation|State"
    )
    EAnimationKey CurrentAnimationKey = EAnimationKey::None;


    /**
     * Animation currently being played by the animation system.
     */
    UPROPERTY(
        BlueprintReadOnly,
        Category = "Animation|State"
    )
    EAnimationKey ActiveAnimationKey = EAnimationKey::None;


    /**
     * Whether the animation system is currently transitioning
     * between animations.
     */
    UPROPERTY(
        BlueprintReadOnly,
        Category = "Animation|State"
    )
    bool bIsBlending = false;


    //=========================================================================
    // Animation Lookup - C++ Only
    //=========================================================================

    /**
     * Finds an animation definition using its gameplay key.
     * Returns nullptr if not found.
     * C++ only - use GetAnimation or TryFindAnimation for Blueprint.
     */
    const FAnimationDefinition* FindAnimation(
        EAnimationKey Key
    ) const;


    //=========================================================================
    // Animation Lookup - Blueprint Safe
    //=========================================================================

    /**
     * Returns an animation definition by key.
     * Returns default (empty) animation if not found.
     * Use HasAnimation to check first.
     */
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnimationDefinition GetAnimation(
        EAnimationKey Key
    ) const;

    /**
     * Returns true if an animation exists for the specified key.
     */
    UFUNCTION(BlueprintPure, Category = "Animation")
    bool HasAnimation(
        EAnimationKey Key
    ) const;

    /**
     * Attempts to find an animation and outputs it if found.
     * Returns true if found, false otherwise.
     */
    UFUNCTION(BlueprintPure, Category = "Animation", meta = (ReturnDisplayName = "Found"))
    bool TryFindAnimation(
        EAnimationKey Key,
        FAnimationDefinition& OutAnimation
    ) const;


    //=========================================================================
    // Animation Requests
    //=========================================================================

    /**
     * Requests an animation by gameplay key.
     *
     * This does NOT directly play the animation.
     * AnimationSystem is responsible for processing the request.
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RequestAnimation(
        EAnimationKey Key
    );


    /**
     * Clears the current animation request.
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ClearAnimationRequest();


    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnimationKey GetCurrentAnimationKey() const
    {
        return CurrentAnimationKey;
    }


    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnimationKey GetActiveAnimationKey() const
    {
        return ActiveAnimationKey;
    }


    //=========================================================================
    // State Management
    //=========================================================================

    /**
     * Called by AnimationSystem once an animation actually becomes active.
     */
    void SetActiveAnimation(
        EAnimationKey Key
    );


    /**
     * Marks the animation as currently blending.
     */
    void SetBlending(
        bool bBlending
    );


    UFUNCTION(BlueprintPure, Category = "Animation|State")
    bool IsAnimationActive(
        EAnimationKey Key
    ) const
    {
        return ActiveAnimationKey == Key;
    }


    UFUNCTION(BlueprintPure, Category = "Animation|State")
    bool IsAnimationRequested(
        EAnimationKey Key
    ) const
    {
        return CurrentAnimationKey == Key;
    }
};
