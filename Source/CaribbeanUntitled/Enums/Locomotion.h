#pragma once

UENUM(BlueprintType)
enum class ELocomotion : uint8
{
    Stand,
    Idle,
    Walk,
    StrafeLeft,
    StrafeRight,
    Sprint,
    Jump,
    Fall,
    Crouch,
    Land
};
