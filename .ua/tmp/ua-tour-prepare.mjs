import fs from "node:fs";

const [graphPath, layersPath, outputPath] = process.argv.slice(2);

if (!graphPath || !layersPath || !outputPath) {
  console.error("Usage: node ua-tour-prepare.mjs <graph> <layers> <output>");
  process.exit(1);
}

const graph = JSON.parse(fs.readFileSync(graphPath, "utf8"));
const layers = JSON.parse(fs.readFileSync(layersPath, "utf8"));
const fileTypes = new Set([
  "file",
  "config",
  "document",
  "service",
  "pipeline",
  "table",
  "schema",
  "resource",
  "endpoint",
]);

const input = {
  nodes: graph.nodes
    .filter((node) => fileTypes.has(node.type))
    .map(({ id, type, name, filePath, summary }) => ({
      id,
      type,
      name,
      filePath,
      summary,
    })),
  edges: graph.edges,
  layers: layers.map(({ id, name, description }) => ({
    id,
    name,
    description,
  })),
};

fs.writeFileSync(outputPath, `${JSON.stringify(input, null, 2)}\n`);
