#pragma once

UENUM(BlueprintType)
enum class ELocomotion : uint8
{
    Stand,
    Idle,
    Walk,
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
