# Animation Pipeline

This document explains how animation works in this project from input or gameplay intent all the way to playback, including how client and server responsibilities are split.

## Big Picture

The animation pipeline is request-driven:

1. A gameplay system decides an animation key.
2. That system publishes a request or calls the animation subsystem directly.
3. `UAnimationSystem` resolves the request against the actor's animation registry.
4. `UAnimationSystem` plays the animation on the actor's mesh using the actor's configured animation asset and blend settings.

The animation system is the only place that actually plays animations.

## Main Pieces

- `UAnimationRegistryComponent`
- `FAnimationDefinition`
- `UAnimationSystem`
- `UCombatAnimationSystem`
- `UMovementAnimationSystem`
- `FAnimationRequestPayload`
- `FMovementPayload`

## What The Registry Does

`UAnimationRegistryComponent` is data only.

It stores an array of `FAnimationDefinition` entries, where each entry maps:

- `EAnimationKey`
- animation asset
- slot enum
- layer
- breakpoint mode
- breakpoint bone name
- blend in time
- blend out time
- playback mode
- play rate
- optional blend curve

The registry does not play anything by itself.

## Where The Assets Come From

There are two ways animations get into the registry:

1. Default setup in `ACharacterActor`
2. Profile-driven overrides in `APlayableActor`

`ACharacterActor` registers the base locomotion and combat keys with default assets and blend settings.

`APlayableActor` loads the animation profile from `UCaribbeanAssetSettings` and overwrites matching registry entries with the profile's assets.

For the Demo Bot, the current movement set includes:

- `Idle`
- `Walk`
- `WalkBackwards`
- `LeftStrafe`
- `RightStrafe`
- `LeftStrafeRun`
- `RightStrafeRun`
- `Run`
- `Sprint`
- `Jump`
- `Fall`
- `Land`

## How Animation Requests Are Made

There are two common request paths:

### Movement

`UMovementAnimationSystem` listens for `FMovementPayload` on the event bus.

It converts locomotion state into an `EAnimationKey` and then calls:

```cpp
UAnimationSystem::RequestAnimationState(Actor, Key);
```

### Combat

`UCombatAnimationSystem` listens for `FAttackStartedPayload`.

It reads the current attack animation key from the combat component and requests the appropriate animation on the animation system.

## How `UAnimationSystem` Works

`UAnimationSystem` is a world subsystem with per-actor, per-layer state.

When `RequestAnimationState` is called:

1. It finds or creates state for the actor and layer.
2. It checks whether the new state is already active.
3. It looks for a transition rule if `bForce` is false.
4. It builds a queue of states to play.
5. It plays each state in order.

## Transition Rules

The animation system supports rules that can:

- play a direct state
- insert intermediate states
- override the final state

That is how the project handles things like:

- landing before returning to idle
- combo-style transitions
- custom movement transitions

## Playback Path

When a state is played, the system:

1. Finds the actor's `UAnimationRegistryComponent`
2. Looks up the requested `EAnimationKey`
3. Uses the actor's mesh and anim instance
4. Plays the sequence as a dynamic montage using the definition's layer and blend settings

If there is no `UAnimInstance`, it falls back to direct mesh playback.

## Layer Masks And Hierarchy

Bone evaluation now stays inside Unreal's native AnimGraph. C++ selects an
animation layer and plays a dynamic montage into its corresponding slot.

The Animation Blueprint must provide the matching slot nodes:

- `lower-body` for full-body locomotion
- `upper-body` for combat and other upper-body overlays
- `face` for facial animation
- `ik` for IK/additive work

Use `Layered Blend Per Bone` in the AnimGraph to combine the upper-body slot
with locomotion. The current character split starts at `Spine`.

This moves hierarchy traversal, pose caching, update-rate optimization, and
parallel animation evaluation back into the engine instead of walking and
writing every bone from gameplay C++.

`FAnimationDefinition::Breakpoint` and
`ACharacterActor::UpperBodyBreakpointBoneName` remain available for asset
compatibility, but the AnimGraph's blend filters now define the runtime mask.

## Skeletal Mesh Runtime

`ACharacter::GetMesh()` is the visible runtime mesh. Its AnimInstance evaluates
locomotion, slots, montages, and layered blends natively.

The old `UPoseableMeshComponent` remains hidden and non-ticking so existing
Blueprint assets do not lose the serialized component, but it is no longer part
of the runtime render path.

## Client And Server Behavior

### Server

The server is authoritative for gameplay state and locomotion state.

For movement:

- `UMovementSystem` runs on the authoritative side.
- It evaluates velocity, input, sprint state, crouch state, jump state, and other movement conditions.
- It calls `ControlComponent->SetReplicatedLocomotion(...)` on the server.
- That replicated locomotion value is then sent to clients.

For combat:

- combat requests are initiated through the combat component and combat animation system
- the server is the side that should own the final authoritative request

### Client

The owning client still contributes local input and camera yaw.

Clients do not directly drive animation playback.

Instead:

- they update control intent on `UControlComponent`
- those values replicate
- the server decides locomotion and animation requests

### Replication Bridge

`UControlComponent::OnRep_ReplicatedLocomotion()` publishes an `FMovementPayload` on the event bus.

That lets the animation side react to replicated locomotion changes without animation code needing to know about networking.

## Why The Blend Settings Matter

Each registry entry carries its own blend in/out values.

Those values are used when the animation system creates the dynamic montage.

That means:

- idle to walk can use one blend speed
- walk to sprint can use another
- upper-body combat can blend differently from locomotion

## Practical Flow Example

### Example: light attack

1. The local player presses the attack input.
2. `UPlayerInputComponent` calls into `UControlComponent`.
3. The combat component requests the attack.
4. `UCombatAnimationSystem` receives the attack start event.
5. It requests the current attack animation key from `UAnimationSystem`.
6. `UAnimationSystem` resolves the key against the registry and plays the montage on the correct layer.

### Example: walk to strafe

1. The player presses a lateral movement key.
2. `UControlComponent` stores that intent.
3. `UMovementSystem` evaluates the intent and chooses the correct locomotion state.
4. `UControlComponent` replicates the locomotion value.
5. `OnRep_ReplicatedLocomotion()` publishes the movement payload.
6. `UMovementAnimationSystem` converts the locomotion state into the right animation key.
7. `UAnimationSystem` plays the registered animation asset.

## Extending The Pipeline

To add a new animation:

1. Add a new `EAnimationKey`.
2. Register the animation in `UAnimationRegistryComponent`.
3. Add a matching profile entry if the playable actor should override it.
4. Add a request path in the gameplay system that owns that behavior.
5. If needed, add a transition rule.

## Notes

- Movement and combat do not play animations directly.
- Input does not play animations directly.
- The animation system owns playback.
- The registry owns data.
- The gameplay systems own the decision of what animation should happen.
