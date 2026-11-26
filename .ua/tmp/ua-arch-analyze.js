const fs = require("fs");
const path = require("path");

const [inputPath, outputPath] = process.argv.slice(2);
if (!inputPath || !outputPath) {
  console.error("Usage: node ua-arch-analyze.js <input.json> <output.json>");
  process.exit(1);
}

try {
  const input = JSON.parse(fs.readFileSync(inputPath, "utf8"));
  const fileNodes = input.fileNodes || [];
  const importEdges = input.importEdges || [];
  const allEdges = input.allEdges || [];
  const nodeById = new Map(fileNodes.map((node) => [node.id, node]));

  function groupFor(filePath) {
    const parts = filePath.split("/");
    if (parts[0] === "Source" && parts[1] === "CaribbeanUntitled") {
      return parts.length > 3 ? parts[2] : "ModuleRoot";
    }
    if (parts[0] === "Source") return "BuildTargets";
    if (parts[0] === "Config") return "Config";
    return parts.length > 1 ? parts[0] : "ProjectRoot";
  }

  const directoryGroups = {};
  const nodeTypeGroups = {};
  const groupById = new Map();
  for (const node of fileNodes) {
    const group = groupFor(node.filePath || node.name || "");
    groupById.set(node.id, group);
    (directoryGroups[group] ||= []).push(node.id);
    (nodeTypeGroups[node.type] ||= []).push(node.id);
  }

  const fanIn = Object.fromEntries(fileNodes.map((n) => [n.id, 0]));
  const fanOut = Object.fromEntries(fileNodes.map((n) => [n.id, 0]));
  const inter = new Map();
  const internal = {};
  const total = {};
  const dependencies = new Map();

  for (const edge of importEdges) {
    if (!nodeById.has(edge.source) || !nodeById.has(edge.target)) continue;
    fanOut[edge.source]++;
    fanIn[edge.target]++;
    const sourceGroup = groupById.get(edge.source);
    const targetGroup = groupById.get(edge.target);
    total[sourceGroup] = (total[sourceGroup] || 0) + 1;
    total[targetGroup] = (total[targetGroup] || 0) + 1;
    if (sourceGroup === targetGroup) {
      internal[sourceGroup] = (internal[sourceGroup] || 0) + 1;
    } else {
      const key = `${sourceGroup}\u0000${targetGroup}`;
      inter.set(key, (inter.get(key) || 0) + 1);
      const depKey = `${sourceGroup}\u0000${targetGroup}`;
      dependencies.set(depKey, (dependencies.get(depKey) || 0) + 1);
    }
  }

  const crossCategoryCounts = new Map();
  for (const edge of allEdges) {
    const source = nodeById.get(edge.source);
    const target = nodeById.get(edge.target);
    if (!source || !target) continue;
    const key = `${source.type}\u0000${target.type}\u0000${edge.type}`;
    crossCategoryCounts.set(key, (crossCategoryCounts.get(key) || 0) + 1);
  }

  const patterns = {
    Actors: "domain",
    Components: "component",
    Systems: "service",
    Scheduler: "service",
    EventBus: "service",
    Events: "types",
    Types: "types",
    Enums: "types",
    UI: "ui",
    Settings: "config",
    Config: "config",
    Docs: "documentation",
    GameMode: "entry",
    ModuleRoot: "entry",
    BuildTargets: "config",
    ProjectRoot: "config"
  };

  const documentation = fileNodes.filter((n) => n.type === "document");
  const documentedGroups = new Set(documentation.map((n) => groupById.get(n.id)));
  const allGroups = Object.keys(directoryGroups);
  const schemaFiles = fileNodes.filter((n) => ["schema", "table", "endpoint"].includes(n.type));

  const result = {
    scriptCompleted: true,
    directoryGroups,
    nodeTypeGroups,
    crossCategoryEdges: [...crossCategoryCounts.entries()].map(([key, count]) => {
      const [fromType, toType, edgeType] = key.split("\u0000");
      return { fromType, toType, edgeType, count };
    }),
    interGroupImports: [...inter.entries()].map(([key, count]) => {
      const [from, to] = key.split("\u0000");
      return { from, to, count };
    }).sort((a, b) => b.count - a.count),
    intraGroupDensity: Object.fromEntries(allGroups.map((group) => [
      group,
      {
        internalEdges: internal[group] || 0,
        totalEdges: total[group] || 0,
        density: total[group] ? (internal[group] || 0) / total[group] : 0
      }
    ])),
    patternMatches: Object.fromEntries(allGroups.map((group) => [group, patterns[group] || "shared"])),
    deploymentTopology: {
      hasDockerfile: false,
      hasCompose: false,
      hasK8s: false,
      hasTerraform: false,
      hasCI: false,
      infraFiles: []
    },
    dataPipeline: {
      schemaFiles: schemaFiles.map((n) => n.filePath),
      migrationFiles: [],
      dataModelFiles: [],
      apiHandlerFiles: []
    },
    docCoverage: {
      groupsWithDocs: documentedGroups.size,
      totalGroups: allGroups.length,
      coverageRatio: allGroups.length ? documentedGroups.size / allGroups.length : 0,
      undocumentedGroups: allGroups.filter((g) => !documentedGroups.has(g))
    },
    dependencyDirection: [...dependencies.entries()].map(([key, count]) => {
      const [dependent, dependsOn] = key.split("\u0000");
      return { dependent, dependsOn, count };
    }).sort((a, b) => b.count - a.count),
    fileStats: {
      totalFileNodes: fileNodes.length,
      filesPerGroup: Object.fromEntries(Object.entries(directoryGroups).map(([k, v]) => [k, v.length])),
      nodeTypeCounts: Object.fromEntries(Object.entries(nodeTypeGroups).map(([k, v]) => [k, v.length]))
    },
    fileFanIn: fanIn,
    fileFanOut: fanOut
  };

  fs.writeFileSync(outputPath, JSON.stringify(result, null, 2) + "\n");
} catch (error) {
  console.error(error.stack || error.message);
  process.exit(1);
}
