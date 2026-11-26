# Caribbean Untitled

`Caribbean Untitled` is an Unreal Engine 5.8 C++ action-game project built around server-authoritative gameplay, component-based characters, and explicitly ordered gameplay systems.

The current production target is PC at 60 FPS, with a working budget of approximately 20 active characters.

## Project Status

This repository is an in-development gameplay foundation rather than a finished game. It currently contains:

- player and NPC character composition
- Enhanced Input integration
- camera-relative movement and locomotion state
- stamina and sprint rules
- perception, brain, and action systems for AI
- phased melee attacks and combat-volume detection
- replicated control, locomotion, stats, and combat state
- request-driven layered animation
- profile-based character, animation, input, and attachment configuration
- modular local-player HUD components

## Requirements

- Unreal Engine 5.8
- Visual Studio 2022 with the Desktop and Game Development with C++ workloads, or a compatible Unreal C++ toolchain
- Windows 10 or Windows 11 for the primary PC target

The project module depends on:

- Core
- CoreUObject
- Engine
- InputCore
- EnhancedInput
- UMG
- Slate
- SlateCore

## Getting Started

1. Install or build the matching Unreal Engine 5.8 version.
2. Generate project files from `CaribbeanUntitled.uproject`.
3. Build the `CaribbeanUntitledEditor` target in Development Editor configuration.
4. Open `CaribbeanUntitled.uproject`.
5. Configure the required project assets and profiles under `UCaribbeanAssetSettings`.
6. Confirm the project collision channels, animation slots, input mappings, startup map, and game mode before running PIE.

Content assets referenced by configuration or C++ must exist in the local Unreal project. Missing meshes, Animation Blueprints, animation sequences, input actions, or maps can allow compilation to succeed while leaving runtime features incomplete.

## Architecture

The project uses three complementary communication styles:

- direct component access for state owned by an actor
- an actor registry for efficient system queries
- a typed event bus for decoupled gameplay notifications

The main runtime structure is:

```text
Actors
  └─ own reusable Components and replicated state

GameplaySystemScheduler
  └─ executes major gameplay systems in an explicit frame order

ActorRegistry
  └─ indexes actors by component type for system queries

EventBusSubsystem
  └─ publishes typed events between otherwise independent systems

World Subsystems
  └─ provide animation, attack timing, setup, and other world-scoped services
```

### Source Layout

| Directory | Responsibility |
| --- | --- |
| `Actors` | Player, NPC, weapon, and shared character actor composition |
| `Components` | Actor-owned control, stats, combat, perception, animation, and move data |
| `Systems` | Movement, AI, combat, animation, camera, stamina, and registry logic |
| `Scheduler` | Ordered gameplay-system execution |
| `EventBus` / `Events` | Typed event transport and payload contracts |
| `Types` / `Enums` | Shared gameplay data and state definitions |
| `Settings` | Profile-driven asset configuration |
| `UI` | Local-player HUD controller and modular widget components |
| `GameMode` | Game-mode setup and default runtime rules |
| `Docs` | Detailed system-specific engineering notes |

## Frame Execution

`UGameplaySystemScheduler` is the primary gameplay orchestrator. It separates work by authority and frame phase.

The current high-level order is:

```text
Client input
    ↓
Server gameplay
    ├─ combat input
    ├─ perception
    ├─ brain
    ├─ action
    └─ combat detection
    ↓
Movement and stamina
    ↓
Client camera
    ↓
Physics / post-physics / transform hooks
```

New continuously evaluated gameplay logic should have an explicit owner and phase. Avoid adding independent actor Tick functions when the work can be event-driven, registered with an existing system, or scheduled centrally.

## Character Composition

`ACharacterActor` provides the shared character foundation. Specialized actors such as the playable character and NPC build on that base.

Character behavior is divided into focused components:

- `UControlComponent` stores input intent, camera yaw, locomotion, and replicated control state.
- `UStatsComponent` owns health, stamina, and related gameplay values.
- `UCombatComponent` owns attack availability and move selection.
- `UAttackStateComponent` tracks the active attack phase and elapsed phase time.
- `UCombatVolumeComponent` represents an attack or hurt volume.
- `UPerceptionComponent` stores sensed targets.
- `UCharacterBrainComponent` stores AI intent.
- `UAnimationRegistryComponent` maps gameplay animation keys to assets and playback settings.
- `UMoveSetComponent` contains the character's available combat moves.

Components own state; systems evaluate groups of actors and coordinate behavior.

## Input and Movement

The local player does not directly implement gameplay outcomes inside input callbacks.

```text
Enhanced Input
    ↓
UPlayerInputComponent
    ↓
UControlComponent intent
    ↓
server RPC / replicated state
    ↓
UMovementSystem
    ↓
replicated locomotion
    ↓
animation request
```

The server remains authoritative for movement-related gameplay decisions. Camera-relative direction uses replicated view yaw so client intent and server evaluation share the same frame of reference.

See [Input Pipeline](Source/CaribbeanUntitled/Docs/InputPipeline.md).

## AI

AI is split into a small decision pipeline:

1. `UPerceptionSystem` updates what an NPC can sense.
2. `UBrainSystem` turns perception and state into intent.
3. `UActionSystem` converts intent into control commands.
4. Movement and combat systems consume those commands through the same component interfaces used by players.

This keeps high-level decisions separate from execution and avoids placing a monolithic behavior loop on each NPC.

For the 20-character target, expensive perception and decision work should remain staggered or frequency-limited. Systems should query the actor registry instead of scanning the entire world every frame.

## Combat

Combat is server-authoritative and phase-based.

```text
attack intent
    ↓
combat request validation
    ↓
USetupAttackSystem
    ↓
FAttackStartedPayload
    ↓
UTimeAttackSystem
    ├─ anticipation
    ├─ active
    └─ recovery
    ↓
UCombatDetectionSystem
    ↓
UCombatSystem
    ↓
UStatsComponent
```

Attack volumes are enabled only during the active phase. Damage and final hit outcomes must be decided on the authority, not trusted from a client.

When adding a move:

1. define its timing and animation data
2. assign its combat volumes
3. include it in the appropriate move set
4. verify anticipation, active, and recovery transitions
5. test repeated overlap, interrupted attacks, destroyed actors, latency, and late join behavior

## Animation and Bone Handling

Animation requests are data-driven:

```text
gameplay state or event
    ↓
movement/combat animation system
    ↓
EAnimationKey
    ↓
UAnimationRegistryComponent
    ↓
UAnimationSystem
    ↓
dynamic montage on the native skeletal mesh
```

The visible runtime mesh is `ACharacter::GetMesh()`. Unreal's native AnimGraph performs pose evaluation, hierarchy traversal, cached-pose reuse, layered blending, parallel animation work, and update-rate optimization.

The previous manual per-bone C++ pose-copy path is not part of runtime presentation. A hidden, non-ticking poseable mesh may remain for serialized Blueprint compatibility, but gameplay code must not copy and write every bone each frame.

The Animation Blueprint is expected to provide these slots:

- `lower-body`
- `upper-body`
- `face`
- `ik`

Use cached poses and `Layered Blend Per Bone` for upper-body actions over locomotion. The current upper-body split begins at `Spine`; skeleton-specific attachment bones remain profile-driven.

IK, aiming, head tracking, and procedural bone corrections belong in the AnimGraph or Control Rig unless a measured requirement proves otherwise.

See [Animation Pipeline](Source/CaribbeanUntitled/Docs/AnimationPipeline.md) and [Profile Setup](Source/CaribbeanUntitled/Docs/Profiles.md).

## Networking Model

The project follows a server-authoritative model:

- clients capture and submit intent
- the server validates gameplay requests
- the server owns combat timing, hit detection, damage, stats, and authoritative locomotion
- replicated state and notifications drive remote presentation
- local-only UI and camera work stay on the owning client

New replicated systems should define:

- who owns the data
- which side may mutate it
- RPC validation and rate limits
- replication conditions
- late-join behavior
- correction or prediction behavior
- behavior when actors or components become invalid mid-operation

Avoid replicating high-frequency values when they can be derived safely from existing authoritative state.

## PC 60 FPS Budget

The total frame budget at 60 FPS is approximately `16.67 ms`. Treat that as a whole-frame budget, not a gameplay-only allowance.

Recommended working targets:

| Area | Guideline |
| --- | --- |
| Game thread | Keep gameplay comfortably below the full frame budget; preserve headroom for rendering and spikes |
| Actor count | Validate with at least 20 active characters, not only one player and one NPC |
| AI | Stagger perception and decision updates; avoid full-rate work for distant or inactive NPCs |
| Combat | Process active attacks only; do not run idle per-character attack loops |
| Queries | Use `UActorRegistry` component indexes instead of repeated world scans |
| Animation | Use native skeletal evaluation, visibility-based ticking, and update-rate optimization |
| Rendering | Profile Lumen, Virtual Shadow Maps, Nanite, translucency, and skeletal meshes on target hardware |
| Memory | Stream large assets and avoid hard references that load unrelated character or level content |

Use Unreal Insights, `stat unit`, `stat game`, `stat gpu`, `stat anim`, `stat net`, and the Session Frontend profiler before accepting an optimization. Measure representative packaged builds; editor timing is not a shipping-performance result.

## Asset Profiles

`UCaribbeanAssetSettings` allows character variants to select data without adding hard-coded branches.

Profile groups include:

- character visuals
- animation sets
- combat attachment bone maps
- control mappings

Use stable profile keys and keep related visual, animation, control, and attachment entries aligned. Skeleton-specific bone names belong in attachment profiles rather than combat-system code.

See [Profile Setup](Source/CaribbeanUntitled/Docs/Profiles.md).

## HUD

The HUD is local-player only and componentized:

- `APlayerHudController` creates the local HUD.
- `UHudWidget` owns the root widget.
- `UHudWidgetComponent` is the extension point for individual HUD features.
- `ULifeHudWidgetComponent` is the current health-display example.

Add new HUD features as focused widget components instead of expanding a single monolithic widget.

See [HUD Widgets](Source/CaribbeanUntitled/Docs/HudWidgets.md).

## Engineering Rules

- Preserve server authority for gameplay outcomes.
- Prefer explicit system phases over uncontrolled Tick ordering.
- Keep state on the actor component that owns it.
- Use weak object references for cached actor collections.
- Validate `UObject`, actor, component, world, and subsystem pointers at asynchronous or event boundaries.
- Unsubscribe event listeners during deinitialization.
- Keep public Blueprint and C++ interfaces stable during refactors unless a migration is planned.
- Use soft references for optional or streamable assets.
- Do not put skeleton-specific bone names into generic gameplay logic.
- Do not add manual per-frame bone traversal to gameplay C++.
- Add profiling evidence for performance-driven complexity.

## Production Readiness Checklist

Before treating a feature as production-ready:

- it builds in Development Editor and packaged Development configurations
- multiplayer behavior is tested with listen and dedicated server modes
- invalid, destroyed, disconnected, and late-joining actors are handled
- public Blueprint references and serialized components remain compatible
- no required asset is referenced only by an unchecked hard-coded path
- gameplay systems do not perform unnecessary idle work
- logs are categorized and do not spam per frame
- representative 20-character encounters meet the PC 60 FPS target
- animation slots, collision channels, maps, profiles, and input assets are validated
- automated tests cover deterministic gameplay rules where practical

## Known Production Gaps

The following areas should be treated as active production work:

- automated gameplay, replication, and performance tests
- continuous integration and packaged-build validation
- target-hardware performance baselines
- formal network bandwidth and RPC-abuse budgets
- asset-reference and circular-dependency audits
- map, collision-channel, and profile validation tooling
- removal or migration of compatibility-only serialized components
- broader error handling and project-specific log categories

## Documentation

- [Input Pipeline](Source/CaribbeanUntitled/Docs/InputPipeline.md)
- [Animation Pipeline](Source/CaribbeanUntitled/Docs/AnimationPipeline.md)
- [Profile Setup](Source/CaribbeanUntitled/Docs/Profiles.md)
- [HUD Widgets](Source/CaribbeanUntitled/Docs/HudWidgets.md)

Keep these documents updated when system ownership, replication rules, animation slots, asset-profile contracts, or frame ordering changes.

