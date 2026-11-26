const fs = require("fs");

const projectRoot = process.argv[2];
const graphPath = `${projectRoot}/.ua/intermediate/assembled-graph.json`;
const outputPath = `${projectRoot}/.ua/intermediate/layers.json`;
const graph = JSON.parse(fs.readFileSync(graphPath, "utf8"));
const fileTypes = new Set([
  "file", "config", "document", "service", "pipeline",
  "table", "schema", "resource", "endpoint"
]);
const fileNodes = graph.nodes.filter((node) => fileTypes.has(node.type));

function area(node) {
  const filePath = node.filePath || "";
  if (filePath.startsWith("Source/CaribbeanUntitled/Actors/") ||
      filePath.startsWith("Source/CaribbeanUntitled/GameMode/")) {
    return "runtime";
  }
  if (filePath.startsWith("Source/CaribbeanUntitled/Components/")) {
    return "components";
  }
  if (filePath.startsWith("Source/CaribbeanUntitled/Systems/") ||
      filePath.startsWith("Source/CaribbeanUntitled/Scheduler/")) {
    return "systems";
  }
  if (filePath.startsWith("Source/CaribbeanUntitled/EventBus/") ||
      filePath.startsWith("Source/CaribbeanUntitled/Events/") ||
      filePath.startsWith("Source/CaribbeanUntitled/Enums/") ||
      filePath.startsWith("Source/CaribbeanUntitled/Types/")) {
    return "contracts";
  }
  if (filePath.startsWith("Source/CaribbeanUntitled/UI/")) {
    return "presentation";
  }
  if (filePath.startsWith("Source/CaribbeanUntitled/Docs/")) {
    return "documentation";
  }
  return "project";
}

const definitions = [
  {
    key: "runtime",
    id: "layer:runtime-actors",
    name: "Runtime Actors",
    description: "Character classes and game-mode composition roots that own Unreal object lifetimes, assemble components, and connect player or NPC instances to the gameplay framework."
  },
  {
    key: "components",
    id: "layer:gameplay-components",
    name: "Gameplay Components",
    description: "Replicated and local actor state for control, stats, combat, perception, animation registries, movesets, and collision volumes consumed by higher-level systems."
  },
  {
    key: "systems",
    id: "layer:gameplay-systems",
    name: "Gameplay Systems",
    description: "World-scoped orchestration for movement, stamina, AI, perception, combat phases, hit detection, camera behavior, and animation requests, including the explicit gameplay scheduler."
  },
  {
    key: "contracts",
    id: "layer:contracts-and-messaging",
    name: "Contracts and Messaging",
    description: "Shared enums, payload structures, combat and animation data types, and the typed world event bus that decouple producers from gameplay-system consumers."
  },
  {
    key: "presentation",
    id: "layer:presentation",
    name: "Presentation",
    description: "Local-player HUD controllers and modular UMG widget components that translate replicated gameplay state into client-side interface output."
  },
  {
    key: "project",
    id: "layer:project-platform",
    name: "Project and Platform",
    description: "Unreal module entry points, target and build rules, project descriptor, engine/game/input configuration, asset settings, and other platform-facing integration."
  },
  {
    key: "documentation",
    id: "layer:documentation",
    name: "Documentation",
    description: "Maintainer guides for input, animation, character profiles, and HUD extension workflows."
  }
];

const layers = definitions.map((definition) => ({
  id: definition.id,
  name: definition.name,
  description: definition.description,
  nodeIds: fileNodes.filter((node) => area(node) === definition.key).map((node) => node.id)
}));

const assigned = layers.flatMap((layer) => layer.nodeIds);
const unique = new Set(assigned);
if (layers.some((layer) => layer.nodeIds.length === 0)) {
  throw new Error("A layer has no file assignments.");
}
if (assigned.length !== fileNodes.length || unique.size !== fileNodes.length) {
  throw new Error(`Layer assignment mismatch: assigned=${assigned.length}, unique=${unique.size}, expected=${fileNodes.length}`);
}

fs.writeFileSync(outputPath, JSON.stringify(layers, null, 2) + "\n");
