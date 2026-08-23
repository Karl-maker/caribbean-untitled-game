# Input Pipeline

This document explains how input works in this project, including how local client input becomes replicated control state and how the server uses that state for movement, animation, and poseable-mesh driven IK.

## Big Picture

Input is split into three layers:

1. Local input capture
2. Control state storage and replication
3. Server-side gameplay evaluation

The player does not directly move the character with input callbacks. The callbacks write intent into `UControlComponent`, and the gameplay systems consume that state.

## Main Pieces

- `UPlayerInputComponent`
- `UControlComponent`
- `UMovementSystem`
- `UCameraSystem`
- `UStaminaSystem`
- `UMovementAnimationSystem`

## Local Input Capture

`UPlayerInputComponent` is the input adapter.

It:

- binds Enhanced Input actions
- listens for movement, look, jump, sprint, crouch, and combat actions
- writes the result into `UControlComponent`

It only binds for player-controlled actors and only on the local controller.

## Movement Input

The movement keys are captured as discrete actions:

- forward
- backward
- left
- right

Those handlers update `UControlComponent::MoveIntent`, which is a local-space 2D vector:

- X = left/right
- Y = forward/back

The input component also binds `Triggered` and `Completed` so the value is set when the action starts and cleared when it ends.

## Look Input

Look input is written into `UControlComponent::LookInput`.

The camera system reads that input and turns it into shared camera yaw.

That yaw is then replicated through `UControlComponent::ViewYaw` so movement can stay camera-relative on both client and server.

## Action Input

Buttons like these are also stored on `UControlComponent`:

- jump
- sprint
- crouch
- aim
- fire
- reload
- light attack
- heavy attack
- block
- dodge
- lock on
- weapon ability
- weapon switch

Most of them are simple intent values that the gameplay systems read every tick.

## Client And Server Behavior

### Client

On the owning client:

- input callbacks fire
- `UPlayerInputComponent` updates `UControlComponent`
- `UControlComponent` sends server RPCs for the replicated fields it owns

The client is responsible for capturing intent, not for deciding gameplay results.

### Server

The server is authoritative for movement and gameplay outcomes.

`UMovementSystem` reads the replicated and local control state and decides:

- speed
- jump state
- crouch state
- locomotion state
- whether sprint is allowed
- which direction the character should move

The server also decides which locomotion state gets replicated back to clients.

## Replication In `UControlComponent`

`UControlComponent` replicates the important control state:

- `MoveIntent`
- `ReplicatedLocomotion`
- jump intent
- sprint intent
- crouch intent
- combat intents
- look input related state
- view yaw

Some setters call server RPCs when the owner is not authoritative.

Examples:

- `SetMoveIntent`
- `SetJumpIntent`
- `SetSprintIntent`
- `SetLightAttackIntent`
- `SetHeavyAttackIntent`
- `SetLookInput`
- `SetViewYaw`

That gives the server a consistent view of what the player is trying to do.

## Movement Evaluation

`UMovementSystem` runs the actual movement logic.

It:

1. reads `MoveIntent`
2. resolves camera-relative direction using `ViewYaw`
3. applies speed
4. applies jump
5. applies movement
6. applies crouch
7. determines locomotion

It publishes the current locomotion state through `SetReplicatedLocomotion`.

Important detail:

- sideways intent can become strafe
- forward/back intent can become walk or walk backwards
- sprint is suppressed for backward motion
- sprint strafe uses separate sprint-strafe states

## Camera And Movement

The camera system and movement system share the same yaw via `UControlComponent`.

That is why the project can support camera-relative movement without needing to rely on controller rotation.

The current implementation keeps the character facing the shared camera yaw so strafing stays stable and does not spin the capsule.

## Combat Input Flow

Combat inputs are captured the same way as movement inputs:

1. `UPlayerInputComponent` handles the action.
2. `UControlComponent` stores the intent or starts the request.
3. Combat code triggers the actual gameplay action.
4. Combat animation is requested through the animation pipeline.

That keeps combat input separate from movement input while still using the same control component as the bridge.

## Movement To Animation

This project does not play locomotion animations directly from input.

Instead:

1. input becomes control intent
2. movement logic determines locomotion
3. locomotion is replicated
4. the replicated locomotion change publishes an event
5. `UMovementAnimationSystem` maps locomotion to `EAnimationKey`
6. `UAnimationSystem` plays the animation

That means the input pipeline and animation pipeline are connected, but not fused together.

## Input To Pose

The input pipeline does not drive bone overrides directly.

Instead:

1. input becomes control intent
2. movement and gameplay systems evaluate that intent
3. animation systems choose the active locomotion and action states
4. `ACharacterActor` samples those states into the poseable mesh for runtime presentation

That keeps control input separate from animation playback while still letting the character present the final layered pose each frame.

## Example Flow

### Move forward

1. Player presses forward.
2. `UPlayerInputComponent` updates `MoveIntent.Y`.
3. `UMovementSystem` sees forward intent and moves the character.
4. It sets locomotion to `Walk`, `Sprint`, or another forward state depending on conditions.
5. The locomotion state replicates.
6. `UMovementAnimationSystem` turns it into the matching animation key.

### Move left while sprinting

1. Player holds sprint and presses left.
2. `MoveIntent.X` becomes negative.
3. `UMovementSystem` classifies the state as a left strafe or sprint strafe.
4. The server replicates that locomotion state.
5. The animation system plays the matching strafe animation.

### Move backwards

1. Player presses backward.
2. `MoveIntent.Y` becomes negative.
3. `UMovementSystem` classifies the state as `WalkBackwards`.
4. Sprint is not selected for that case.

## Adding A New Input

To add a new player input:

1. Add a `UInputAction` field to `UPlayerInputComponent`.
2. Bind it in `BindInput`.
3. Write the result into `UControlComponent`.
4. If the action affects gameplay state, add a server RPC or replicated intent as needed.
5. Teach the relevant gameplay system how to consume it.

## Adding A New Movement Direction

To add a new movement direction:

1. Add or reuse a locomotion enum value.
2. Teach `UMovementSystem` how to classify the movement.
3. Replicate that locomotion state through `UControlComponent`.
4. Map it in `UMovementAnimationSystem`.
5. Add the animation registry entry.

## Notes

- Input callbacks are local.
- Gameplay decisions are authoritative on the server.
- Animation playback is handled by the animation subsystem.
- The control component is the bridge between the two worlds.
- The IK system reads direct component state instead of an IK event payload.
