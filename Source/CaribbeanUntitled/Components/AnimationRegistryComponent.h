//
// AnimationRegistryComponent.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimTypes.h"
#include "../Enums/AnimationBlend.h"
#include "AnimationRegistryComponent.generated.h"


// ============================================================================
// Animation Keys
// ============================================================================

/**
 * Standardized animation keys used by gameplay systems.
 *
 * Gameplay systems reference these keys instead of knowing which
 * animation assets a particular character uses.
 */
UENUM(BlueprintType)
enum class EAnimationKey : uint8
{
    None        UMETA(DisplayName = "None"),

    // Locomotion
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    WalkBackwards UMETA(DisplayName = "Walk Backwards"),
    LeftStrafe  UMETA(DisplayName = "Left Strafe"),
    RightStrafe UMETA(DisplayName = "Right Strafe"),
    LeftStrafeRun  UMETA(DisplayName = "Left Strafe Run"),
    RightStrafeRun UMETA(DisplayName = "Right Strafe Run"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),

    // Airborne
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),

    // Combat - Animations for all different attacks
    LightLeftPunch  UMETA(DisplayName = "Light Left Punch"),
    LightRightPunch UMETA(DisplayName = "Light Right Punch"),

    Hit         UMETA(DisplayName = "Hit"),
    Death       UMETA(DisplayName = "Death"),

    // Interaction
    Interact    UMETA(DisplayName = "Interact"),
    Mount       UMETA(DisplayName = "Mount"),
    Dismount    UMETA(DisplayName = "Dismount"),
    
    Crouch    UMETA(DisplayName = "Crouch")
};


// ============================================================================
// Animation Playback
// ============================================================================

/**
 * Defines how an animation should behave when played.
 */
UENUM(BlueprintType)
enum class EAnimationPlaybackMode : uint8
{
    Loop        UMETA(DisplayName = "Loop"),
    Once        UMETA(DisplayName = "Play Once"),
    Hold        UMETA(DisplayName = "Hold Last Frame")
};

UENUM(BlueprintType)
enum class EAnimationLayer : uint8
{
    FullBody    UMETA(DisplayName = "Full Body"),
    UpperBody   UMETA(DisplayName = "Upper Body"),
    Additive    UMETA(DisplayName = "Additive"),
    Face        UMETA(DisplayName = "Face"),
    IK          UMETA(DisplayName = "IK")
};

UENUM(BlueprintType)
enum class EAnimationSlot : uint8
{
    FullBody    UMETA(DisplayName = "Full Body"),
    UpperBody   UMETA(DisplayName = "Upper Body"),
    Face        UMETA(DisplayName = "Face"),
    IK          UMETA(DisplayName = "IK")
};

UENUM(BlueprintType)
enum class EAnimationBreakpoint : uint8
{
    FullBody        UMETA(DisplayName = "Full Body"),
    SpineAndDown    UMETA(DisplayName = "Spine And Down"),
    SpineAndUp      UMETA(DisplayName = "Spine And Up")
};


// ============================================================================
// Animation Definition
// ============================================================================

/**
 * Defines the animation associated with a gameplay animation key.
 *
 * This struct contains configuration only.
 * It does not play or control the animation.
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
    
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Layering"
    )
    EAnimationSlot Slot = EAnimationSlot::FullBody;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Layering"
    )
    EAnimationLayer Layer = EAnimationLayer::FullBody;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Layering"
    )
    EAnimationBreakpoint Breakpoint = EAnimationBreakpoint::FullBody;
    
    UPROPERTY(EditAnywhere)
    TObjectPtr<UAnimSequence> Animation = nullptr;


    /**
     * How the animation should play.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Playback"
    )
    EAnimationPlaybackMode PlaybackMode =
        EAnimationPlaybackMode::Loop;


    /**
     * Playback speed multiplier.
     *
     * 1.0 = normal speed
     * 2.0 = twice as fast
     * 0.5 = half speed
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Playback",
        meta = (ClampMin = "0.0")
    )
    float PlayRate = 1.0f;


    /**
     * Starting position within the animation.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Playback",
        meta = (ClampMin = "0.0")
    )
    float StartPosition = 0.0f;


    /**
     * Number of times the animation should repeat.
     *
     * -1 = infinite
     *  1 = play once
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Playback",
        meta = (ClampMin = "-1")
    )
    int32 LoopCount = -1;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Layering"
    )
    EAnimationBlendMode BlendMode = EAnimationBlendMode::FullBody;
    
    /**
     * Time required to blend into this animation.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Blend",
        meta = (ClampMin = "0.0")
    )
    float BlendInTime = 0.15f;


    /**
     * Time required to blend out of this animation.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Blend",
        meta = (ClampMin = "0.0")
    )
    float BlendOutTime = 0.15f;


    /**
     * Optional curve controlling the blend.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation|Blend"
    )
    TObjectPtr<UCurveFloat> BlendCurve = nullptr;


    /**
     * Optional description of the animation.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation"
    )
    FText Description;
};


// ============================================================================
// Animation Registry Component
// ============================================================================

/**
 * Component that maps gameplay animation keys to animation definitions.
 *
 * This component is DATA ONLY.
 *
 * It does not:
 *
 * - Play animations
 * - Request animations
 * - Track active animations
 * - Track current animations
 * - Modify the SkeletalMeshComponent
 * - Execute animation logic
 *
 * Gameplay systems use EAnimationKey values.
 *
 * AnimationSystem uses this component to resolve those keys into
 * Unreal animation assets and their playback configuration.
 */
UCLASS(
    ClassGroup = (Gameplay),
    BlueprintType,
    Blueprintable,
    meta = (BlueprintSpawnableComponent)
)
class CARIBBEANUNTITLED_API UAnimationRegistryComponent
    : public UActorComponent
{
    GENERATED_BODY()

public:

    UAnimationRegistryComponent();


    //=========================================================================
    // Registry
    //=========================================================================

    /**
     * All animations available to this Actor.
     *
     * Each entry represents:
     *
     *     AnimationKey -> AnimationDefinition
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Animation Registry"
    )
    TArray<FAnimationDefinition> Animations;


    //=========================================================================
    // Lookup
    //=========================================================================

    /**
     * Finds an animation definition by gameplay key.
     *
     * Returns nullptr if the key is not registered.
     *
     * C++ only.
     */
    const FAnimationDefinition* FindAnimation(
        EAnimationKey Key
    ) const;


    /**
     * Gets an animation definition by gameplay key.
     *
     * Returns an empty definition if the key is not registered.
     */
    UFUNCTION(
        BlueprintPure,
        Category = "Animation Registry"
    )
    FAnimationDefinition GetAnimation(
        EAnimationKey Key
    ) const;


    /**
     * Returns true if the key has a registered animation.
     */
    UFUNCTION(
        BlueprintPure,
        Category = "Animation Registry"
    )
    bool HasAnimation(
        EAnimationKey Key
    ) const;


    /**
     * Attempts to find an animation definition.
     *
     * Returns true when the animation exists.
     */
    UFUNCTION(
        BlueprintPure,
        Category = "Animation Registry",
        meta = (ReturnDisplayName = "Found")
    )
    bool TryFindAnimation(
        EAnimationKey Key,
        FAnimationDefinition& OutAnimation
    ) const;
};
