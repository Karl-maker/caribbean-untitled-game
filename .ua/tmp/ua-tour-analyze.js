const fs = require("node:fs");

const [inputPath, outputPath] = process.argv.slice(2);

if (!inputPath || !outputPath) {
  console.error("Usage: node ua-tour-analyze.js <input> <output>");
  process.exit(1);
}

try {
  const input = JSON.parse(fs.readFileSync(inputPath, "utf8"));
  const nodes = input.nodes ?? [];
  const edges = input.edges ?? [];
  const layers = input.layers ?? [];
  const byId = new Map(nodes.map((node) => [node.id, node]));
  const fanIn = new Map(nodes.map((node) => [node.id, 0]));
  const fanOut = new Map(nodes.map((node) => [node.id, 0]));

  for (const edge of edges) {
    if (byId.has(edge.source) && byId.has(edge.target)) {
      fanOut.set(edge.source, (fanOut.get(edge.source) ?? 0) + 1);
      fanIn.set(edge.target, (fanIn.get(edge.target) ?? 0) + 1);
    }
  }

  const rank = (counts, field) =>
    [...counts.entries()]
      .map(([id, count]) => ({
        id,
        [field]: count,
        name: byId.get(id)?.name ?? id,
      }))
      .sort((a, b) => b[field] - a[field] || a.id.localeCompare(b.id))
      .slice(0, 20);

  const fanInRanking = rank(fanIn, "fanIn");
  const fanOutRanking = rank(fanOut, "fanOut");
  const topFanOutCutoff =
    [...fanOut.values()].sort((a, b) => b - a)[
      Math.max(0, Math.ceil(nodes.length * 0.1) - 1)
    ] ?? 0;
  const bottomFanInCutoff =
    [...fanIn.values()].sort((a, b) => a - b)[
      Math.max(0, Math.ceil(nodes.length * 0.25) - 1)
    ] ?? 0;
  const entryNames = new Set([
    "index.ts", "index.js", "main.ts", "main.js", "app.ts", "app.js",
    "server.ts", "server.js", "mod.rs", "main.go", "main.py", "main.rs",
    "manage.py", "app.py", "wsgi.py", "asgi.py", "run.py", "__main__.py",
    "application.java", "main.java", "program.cs", "config.ru", "index.php",
    "app.swift", "application.kt", "main.cpp", "main.c",
  ]);

  const entryPointCandidates = nodes
    .map((node) => {
      const name = (node.name ?? "").toLowerCase();
      const path = node.filePath ?? "";
      const depth = path.split("/").filter(Boolean).length;
      let score = 0;
      if (node.type === "file") {
        if (entryNames.has(name)) score += 3;
        if (depth <= 2) score += 1;
        if ((fanOut.get(node.id) ?? 0) >= topFanOutCutoff) score += 1;
        if ((fanIn.get(node.id) ?? 0) <= bottomFanInCutoff) score += 1;
      } else if (node.type === "document") {
        if (path === "README.md") score += 5;
        else if (depth === 1 && name.endsWith(".md")) score += 2;
      }
      return { id: node.id, score, name: node.name, summary: node.summary, type: node.type };
    })
    .filter((candidate) => candidate.score > 0)
    .sort((a, b) => b.score - a.score || a.id.localeCompare(b.id))
    .slice(0, 5);

  const codeStart =
    entryPointCandidates.find((candidate) => candidate.type === "file")?.id ??
    nodes.find((node) => node.type === "file")?.id ??
    null;
  const adjacency = new Map(nodes.map((node) => [node.id, []]));
  for (const edge of edges) {
    if (
      (edge.type === "imports" || edge.type === "calls") &&
      byId.has(edge.source) &&
      byId.has(edge.target)
    ) {
      adjacency.get(edge.source).push(edge.target);
    }
  }

  const order = [];
  const depthMap = {};
  if (codeStart) {
    const queue = [codeStart];
    depthMap[codeStart] = 0;
    while (queue.length > 0) {
      const current = queue.shift();
      order.push(current);
      for (const next of adjacency.get(current) ?? []) {
        if (depthMap[next] === undefined) {
          depthMap[next] = depthMap[current] + 1;
          queue.push(next);
        }
      }
    }
  }
  const byDepth = {};
  for (const id of order) {
    const depth = String(depthMap[id]);
    (byDepth[depth] ??= []).push(id);
  }

  const inventoryEntry = (node) => ({
    id: node.id,
    name: node.name,
    type: node.type,
    summary: node.summary,
  });
  const nonCodeFiles = {
    documentation: nodes.filter((node) => node.type === "document").map(inventoryEntry),
    infrastructure: nodes
      .filter((node) => ["service", "pipeline", "resource"].includes(node.type))
      .map(inventoryEntry),
    data: nodes
      .filter((node) => ["table", "schema", "endpoint"].includes(node.type))
      .map(inventoryEntry),
    config: nodes.filter((node) => node.type === "config").map(inventoryEntry),
  };

  const relationshipSet = new Set(
    edges
      .filter((edge) => ["imports", "calls"].includes(edge.type))
      .map((edge) => `${edge.source}\u0000${edge.target}`)
  );
  const clusters = [];
  const clustered = new Set();
  for (const edge of edges) {
    if (
      !["imports", "calls"].includes(edge.type) ||
      !byId.has(edge.source) ||
      !byId.has(edge.target) ||
      !relationshipSet.has(`${edge.target}\u0000${edge.source}`)
    ) {
      continue;
    }
    const key = [edge.source, edge.target].sort().join("\u0000");
    if (clustered.has(key)) continue;
    clustered.add(key);
    const clusterNodes = new Set([edge.source, edge.target]);
    for (const candidate of nodes) {
      if (clusterNodes.has(candidate.id)) continue;
      let connections = 0;
      for (const member of clusterNodes) {
        if (
          relationshipSet.has(`${candidate.id}\u0000${member}`) ||
          relationshipSet.has(`${member}\u0000${candidate.id}`)
        ) {
          connections += 1;
        }
      }
      if (connections >= 2 && clusterNodes.size < 5) clusterNodes.add(candidate.id);
    }
    let edgeCount = 0;
    for (const graphEdge of edges) {
      if (clusterNodes.has(graphEdge.source) && clusterNodes.has(graphEdge.target)) {
        edgeCount += 1;
      }
    }
    clusters.push({ nodes: [...clusterNodes], edgeCount });
  }
  clusters.sort((a, b) => b.edgeCount - a.edgeCount);

  const nodeSummaryIndex = Object.fromEntries(
    nodes.map((node) => [
      node.id,
      { name: node.name, type: node.type, summary: node.summary },
    ])
  );

  const result = {
    scriptCompleted: true,
    entryPointCandidates,
    fanInRanking,
    fanOutRanking,
    bfsTraversal: { startNode: codeStart, order, depthMap, byDepth },
    nonCodeFiles,
    clusters: clusters.slice(0, 10),
    layers: { count: layers.length, list: layers },
    nodeSummaryIndex,
    totalNodes: nodes.length,
    totalEdges: edges.length,
  };

  fs.writeFileSync(outputPath, `${JSON.stringify(result, null, 2)}\n`);
} catch (error) {
  console.error(error instanceof Error ? error.stack : String(error));
  process.exit(1);
}
