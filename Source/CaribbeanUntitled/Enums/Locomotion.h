#pragma once

UENUM(BlueprintType)
enum class ELocomotion : uint8
{
    Stand,
    Idle,
    Walk,
    WalkBackwards,
    StrafeLeft,
    StrafeRight,
    SprintStrafeLeft,
    SprintStrafeRight,
    Sprint,
    Jump,
    Fall,
    Crouch,
    Land
};
