import fs from "node:fs";
import path from "node:path";

const root = "/Users/family/Documents/Software Projects/is-ed/caribbean-untitled-game";
const ua = path.join(root, ".ua");
const batches = JSON.parse(fs.readFileSync(path.join(ua, "intermediate/batches.json"), "utf8"));

const summaries = {
  "Source/CaribbeanUntitled/Actors/CharacterActor.h": "Declares the shared networked character base, its gameplay components, profile keys, animation compatibility surface, and replicated visual setup.",
  "Source/CaribbeanUntitled/Actors/NonPlayableCharacter.cpp": "Constructs AI-controlled characters with brain and perception components configured for server-side decision making.",
  "Source/CaribbeanUntitled/Actors/NonPlayableCharacter.h": "Declares the NPC specialization that owns perception and brain state used by the AI systems.",
  "Source/CaribbeanUntitled/Actors/PlayableActor.h": "Declares the player-controlled character specialization with camera, input, and profile-driven setup.",
  "Source/CaribbeanUntitled/GameMode/CaribbeanGameMode.cpp": "Implements authoritative player startup, fallback pawn spawning, possession, and game-mode defaults.",
  "Source/CaribbeanUntitled/GameMode/CaribbeanGameMode.h": "Declares the authoritative game mode and player-start lifecycle overrides.",
  "Source/CaribbeanUntitled/UI/HudWidget.cpp": "Builds and rebuilds the root UMG HUD container, delegating individual panels to modular HUD components.",
  "Source/CaribbeanUntitled/UI/HudWidget.h": "Declares the root user widget and the component collection used to compose the player HUD.",
  "Source/CaribbeanUntitled/UI/HudWidgetComponent.h": "Defines the lightweight extension interface used by modular C++ HUD features.",
  "Source/CaribbeanUntitled/UI/LifeHudWidgetComponent.cpp": "Builds the health presentation widget and binds its display to the controlled pawn's stats component.",
  "Source/CaribbeanUntitled/UI/LifeHudWidgetComponent.h": "Declares tunable health HUD layout, texture, and stats binding state.",
  "Source/CaribbeanUntitled/UI/PlayerHudController.cpp": "Creates the HUD only for local players and installs the default modular life display.",
  "Source/CaribbeanUntitled/UI/PlayerHudController.h": "Declares the local player controller responsible for owning the root HUD widget.",
  "Source/CaribbeanUntitled/Components/CharacterBrainComponent.cpp": "Implements mutation and reset operations for the NPC's current intent, feeling, target, and effort.",
  "Source/CaribbeanUntitled/Components/CharacterBrainComponent.h": "Defines AI personality traits, intent state, and the actor component that stores brain decisions.",
  "Source/CaribbeanUntitled/Components/PerceptionComponent.cpp": "Implements storage updates for perceived actors and ranked threats.",
  "Source/CaribbeanUntitled/Components/PerceptionComponent.h": "Defines perception records, sensory tuning, threat memory, and component-owned observation state.",
  "Source/CaribbeanUntitled/Scheduler/GameplaySystemScheduler.cpp": "Initializes gameplay subsystems and executes them in explicit input, gameplay, movement, physics, post-physics, and transform phases.",
  "Source/CaribbeanUntitled/Scheduler/GameplaySystemScheduler.h": "Declares the world scheduler, phase contract, subsystem references, and frame-level profiling statistics.",
  "Source/CaribbeanUntitled/Systems/ActionSystem.cpp": "Implements interval-driven AI behaviors such as idle, wander, follow, investigate, attack, and flee, translating brain intent into control intent.",
  "Source/CaribbeanUntitled/Systems/ActionSystem.h": "Defines the AI action context, blackboard, behavior interface, per-actor state, and action-system API.",
  "Source/CaribbeanUntitled/Systems/BrainSystem.cpp": "Evaluates staggered NPC decisions from personality and perception, then publishes meaningful intent changes.",
  "Source/CaribbeanUntitled/Systems/BrainSystem.h": "Declares brain decision payloads, per-NPC think scheduling, and intent evaluation helpers.",
  "Source/CaribbeanUntitled/Systems/PerceptionSystem.cpp": "Implements spatially partitioned, interval-driven sight and threat evaluation with line-of-sight checks.",
  "Source/CaribbeanUntitled/Systems/PerceptionSystem.h": "Declares perception scheduling, spatial-grid caches, sight tests, and per-perceiver evaluation state.",
};

const tagsFor = (file) => {
  if (file.includes("/UI/")) return ["ui", "umg", "component", "player-facing"];
  if (file.includes("/Actors/")) return ["actor", "gameplay", "networking", "character"];
  if (file.includes("/GameMode/")) return ["game-mode", "authority", "player-lifecycle"];
  if (file.includes("/Scheduler/")) return ["scheduler", "orchestration", "performance", "world-subsystem"];
  if (file.includes("Perception")) return ["ai", "perception", "performance", "world-subsystem"];
  if (file.includes("Brain")) return ["ai", "decision-making", "state-management"];
  if (file.includes("Action")) return ["ai", "behavior", "control-intent", "world-subsystem"];
  return ["gameplay", "component", "unreal-engine"];
};

const functionSummary = (name, file) => {
  const stem = path.basename(file);
  const special = {
    Initialize: "Acquires and prepares the subsystems required by the scheduler.",
    Deinitialize: "Releases scheduler-owned subsystem state and cached references.",
    Tick: "Advances this system's scheduled work for the current frame.",
    ThinkFor: "Evaluates one NPC's current behavior or decision state.",
    DecideIntent: "Selects an NPC intent from perception, personality, and current state.",
    EvaluatePerceiver: "Builds visible-actor and threat results for one perceiver.",
    IsInSightCone: "Tests range and angular visibility against the configured sight cone.",
    HasLineOfSight: "Performs the world trace used to confirm unobstructed visibility.",
    RebuildHud: "Recreates the modular HUD hierarchy and installs configured components.",
    BuildWidget: "Constructs the health HUD widget hierarchy and its data binding.",
    CreateHud: "Creates and attaches the local player's root HUD widget.",
    HandleStartingNewPlayer_Implementation: "Ensures a joining player receives and possesses a valid pawn.",
    RunTransition: "Runs behavior exit and entry hooks when an NPC intent changes.",
    PickNewTarget: "Chooses a new wander destination using per-actor randomized state."
  };
  return special[name] ?? `Implements ${name} as part of ${stem}'s gameplay responsibility.`;
};

function complexity(lines) {
  if (lines > 200) return "complex";
  if (lines >= 50) return "moderate";
  return "simple";
}

function buildCodeBatch(batchIndex) {
  const batch = batches.batches.find((value) => value.batchIndex === batchIndex);
  const extract = JSON.parse(fs.readFileSync(path.join(ua, `tmp/ua-file-extract-results-${batchIndex}.json`), "utf8"));
  const resultByPath = new Map(extract.results.map((result) => [result.path, result]));
  const nodes = [];
  const edges = [];
  const ids = new Set();

  const addNode = (node) => {
    if (!ids.has(node.id)) {
      ids.add(node.id);
      nodes.push(node);
      return true;
    }
    return false;
  };

  for (const file of batch.files) {
    const result = resultByPath.get(file.path);
    const fileId = `file:${file.path}`;
    addNode({
      id: fileId,
      type: "file",
      name: path.basename(file.path),
      filePath: file.path,
      summary: summaries[file.path] ?? `Provides ${path.basename(file.path)} implementation for the CaribbeanUntitled runtime module.`,
      tags: tagsFor(file.path),
      complexity: complexity(result?.nonEmptyLines ?? file.sizeLines),
      languageNotes: "Unreal Engine reflection macros and UObject ownership rules define the public runtime surface."
    });

    for (const imported of batch.batchImportData[file.path] ?? []) {
      edges.push({source: fileId, target: `file:${imported}`, type: "imports", direction: "forward", weight: 0.7});
    }

    for (const fn of result?.functions ?? []) {
      const span = fn.endLine - fn.startLine + 1;
      if (span < 10) continue;
      const id = `function:${file.path}:${fn.name}`;
      if (!addNode({
        id,
        type: "function",
        name: fn.name,
        filePath: file.path,
        lineRange: [fn.startLine, fn.endLine],
        summary: functionSummary(fn.name, file.path),
        tags: [...tagsFor(file.path).slice(0, 2), "function"],
        complexity: complexity(span)
      })) continue;
      edges.push({source: fileId, target: id, type: "contains", direction: "forward", weight: 1.0});
      edges.push({source: fileId, target: id, type: "exports", direction: "forward", weight: 0.8});
    }

    for (const cls of result?.classes ?? []) {
      const span = cls.endLine - cls.startLine + 1;
      if (span < 20 && (cls.methods?.length ?? 0) < 2) continue;
      const id = `class:${file.path}:${cls.name}`;
      if (!addNode({
        id,
        type: "class",
        name: cls.name,
        filePath: file.path,
        lineRange: [cls.startLine, cls.endLine],
        summary: `${cls.name} groups ${tagsFor(file.path)[0]} state and behavior used by the runtime.`,
        tags: [...tagsFor(file.path).slice(0, 2), "class"],
        complexity: complexity(span)
      })) continue;
      edges.push({source: fileId, target: id, type: "contains", direction: "forward", weight: 1.0});
      edges.push({source: fileId, target: id, type: "exports", direction: "forward", weight: 0.8});
    }
  }
  return {batch, nodes, edges};
}

function writeFragment(batchIndex, graph) {
  const outputDir = path.join(ua, "intermediate");
  const old = fs.readdirSync(outputDir).filter((name) => name.startsWith(`batch-${batchIndex}-part-`));
  for (const name of old) fs.unlinkSync(path.join(outputDir, name));
  const needsSplit = graph.nodes.length > 60 || graph.edges.length > 120;
  if (!needsSplit) {
    fs.writeFileSync(path.join(outputDir, `batch-${batchIndex}.json`), JSON.stringify({nodes: graph.nodes, edges: graph.edges}, null, 2) + "\n");
    return;
  }
  const parts = Math.ceil(Math.max(graph.nodes.length / 60, graph.edges.length / 120));
  const paths = [...graph.batch.files.map((file) => file.path)].sort();
  const chunkSize = Math.ceil(paths.length / parts);
  for (let index = 0; index < parts; index++) {
    const selected = new Set(paths.slice(index * chunkSize, (index + 1) * chunkSize));
    const nodes = graph.nodes.filter((node) => selected.has(node.filePath));
    const nodeIds = new Set(nodes.map((node) => node.id));
    const edges = graph.edges.filter((edge) => nodeIds.has(edge.source));
    fs.writeFileSync(path.join(outputDir, `batch-${batchIndex}-part-${index + 1}.json`), JSON.stringify({nodes, edges}, null, 2) + "\n");
  }
}

for (const batchIndex of [4, 5]) {
  writeFragment(batchIndex, buildCodeBatch(batchIndex));
}

const configBatch = batches.batches.find((value) => value.batchIndex === 6);
const configSummaries = {
  "Config/DefaultEditor.ini": "Defines shared Unreal Editor asset-preview scene profiles and editor-only visualization defaults.",
  "Config/DefaultEngine.ini": "Configures maps, game mode, desktop rendering, Lumen, ray tracing, Substrate, shader models, audio, and platform RHIs.",
  "Config/DefaultGame.ini": "Defines project identity plus profile-driven character visuals, animations, controls, and playable-actor defaults.",
  "Config/DefaultInput.ini": "Configures legacy axis properties, Enhanced Input classes, mouse capture, motion controls, and console input."
};
const configNodes = configBatch.files.map((file) => ({
  id: `config:${file.path}`,
  type: "config",
  name: path.basename(file.path),
  filePath: file.path,
  summary: configSummaries[file.path],
  tags: ["configuration", "unreal-engine", file.path.includes("Input") ? "input" : file.path.includes("Engine") ? "rendering" : "project-settings"],
  complexity: complexity(file.sizeLines),
  languageNotes: "Unreal INI sections bind configuration directly to engine and project UObject classes."
}));
fs.writeFileSync(path.join(ua, "intermediate/batch-6.json"), JSON.stringify({nodes: configNodes, edges: []}, null, 2) + "\n");
