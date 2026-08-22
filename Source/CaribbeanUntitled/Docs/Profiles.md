# Profile Setup Guide

This project uses keyed asset profiles in `UCaribbeanAssetSettings` so characters can swap visuals, animations, combat bone maps, and input mappings without code changes.

## What A Profile Key Does

Each profile type uses a `ProfileKey` such as `DEMO_FEMALE` or `HUMANOID_ANIMATIONS`.

The character reads the key and looks up the matching entry in the project settings:

- `CharacterVisualProfiles`
- `AnimationProfiles`
- `CombatAttachmentProfiles`
- `ControlMappingProfiles`

Use the same key across related profiles when they belong to the same character setup.

## Visual Profiles

Visual profiles control the skeletal mesh and animation blueprint used by a character.

### Fields

- `ProfileKey`
- `SkeletalMesh`
- `AnimBlueprintClass`

### How To Add One

1. Open `UCaribbeanAssetSettings` in Project Settings.
2. Add a new entry under `CharacterVisualProfiles`.
3. Set `ProfileKey` to something stable, such as `DEMO_FEMALE`.
4. Assign the skeletal mesh asset.
5. Assign the animation blueprint class for that mesh.

### Example

- `DEMO_FEMALE`
- Skeletal mesh: female humanoid mesh
- Anim blueprint: humanoid animation blueprint

## Animation Profiles

Animation profiles map gameplay animation keys to the actual animation assets.

### Fields

- `ProfileKey`
- `Entries`

Each entry contains:

- animation key
- animation asset
- blend mode
- optional blend curve

### How To Add One

1. Open `UCaribbeanAssetSettings`.
2. Add a new entry under `AnimationProfiles`.
3. Set `ProfileKey` to a stable animation set name, such as `HUMANOID_ANIMATIONS`.
4. Add one entry per gameplay animation you want to override.
5. Match each entry’s animation key to the gameplay enum value used by the systems.

### Example

- `HUMANOID_ANIMATIONS`
- Idle -> idle animation
- Walk -> walk animation
- WalkBackwards -> walk backwards animation
- LeftStrafe -> left strafe animation
- RightStrafe -> right strafe animation
- LeftStrafeRun -> left strafe run animation
- RightStrafeRun -> right strafe run animation
- Run -> run animation
- Sprint -> sprint animation
- Jump -> jump animation
- Fall -> fall animation
- Land -> land animation

## Combat Attachment Profiles

Combat attachment profiles map body parts to bone names.

This is the profile that lets you swap skeletons later without changing code.

### Fields

- `ProfileKey`
- `BoneNamesByBodyPart`

The body part keys come from `EBodyPart`, such as:

- `Head`
- `Torso`
- `LeftHand`
- `RightHand`
- `LeftFoot`
- `RightFoot`

### How To Add One

1. Open `UCaribbeanAssetSettings`.
2. Add a new entry under `CombatAttachmentProfiles`.
3. Set `ProfileKey` to match the character visual profile key, such as `DEMO_FEMALE`.
4. Fill in the `BoneNamesByBodyPart` map for each body part.
5. Use the exact bone names from the active skeletal mesh.

### Example

For `DEMO_FEMALE`:

- `Head` -> `Head`
- `Torso` -> `Spine2`
- `LeftHand` -> `LeftHand`
- `RightHand` -> `RightHand`
- `LeftFoot` -> `LeftFoot`
- `RightFoot` -> `RightFoot`

If the skeleton changes later, update only the profile, not the combat code.

## Control Mapping Profiles

Control mapping profiles define the input assets for a control scheme.

### Fields

- `ProfileKey`
- `DefaultMappingContext`
- `MoveForwardAction`
- `MoveBackwardAction`
- `MoveLeftAction`
- `MoveRightAction`
- `LookAction`
- `JumpAction`
- `SprintAction`
- `CrouchAction`

### How To Add One

1. Open `UCaribbeanAssetSettings`.
2. Add a new entry under `ControlMappingProfiles`.
3. Set `ProfileKey` to something like `CONTROL_MAPPINGS_DESKTOP`.
4. Assign the input mapping context and action assets.

## Character Setup

`ACharacterActor` uses these keys:

- `CharacterVisualProfileKey`
- `CombatAttachmentProfileKey`

`APlayableActor` uses:

- `AnimationProfileKey`
- `ControlMappingProfileKey`

For a new character variant, keep the keys aligned unless you intentionally want different behavior.

## Recommended Workflow

1. Create or import the skeletal mesh.
2. Confirm the bone names on that mesh.
3. Add a `CharacterVisualProfile` with the mesh and anim blueprint.
4. Add a matching `CombatAttachmentProfile` with the bone map.
5. Add an `AnimationProfile` for the animation set used by that rig.
6. Add a `ControlMappingProfile` if the character needs a new input layout.
7. Set the character keys to the matching `ProfileKey` values.

## Notes

- The combat system uses the generated combat shapes, not the character bones directly.
- If a body part is missing from the active combat attachment profile, the character will now log an error instead of silently guessing a bone name.
- Keep profile keys stable so blueprints and levels do not need code updates when you swap assets.
