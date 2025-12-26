#!/usr/bin/env node

import { copyFileSync, existsSync, mkdirSync } from "fs";
import { dirname, join, resolve } from "path";
import { fileURLToPath } from "url";

const __dirname = dirname(fileURLToPath(import.meta.url));
const workspaceRoot = resolve(__dirname, "../../..");
const wasmBuildDir = resolve(
  workspaceRoot,
  "build_wasm/modules/typescript/dist"
);
const distDir = resolve(__dirname, "../dist");
const srcDir = resolve(__dirname, "..");
const licensePath = resolve(workspaceRoot, "LICENSE");

if (!existsSync(distDir)) {
  mkdirSync(distDir, { recursive: true });
}

const filesToCopy = [
  "l2encdec.browser.js",
  "l2encdec.node.js",
  "l2encdec.js.d.ts"
];

let copied = 0;
let errors = 0;

for (const file of filesToCopy) {
  const srcPath = join(wasmBuildDir, file);
  const dstPath = join(distDir, file);

  if (!existsSync(srcPath)) {
    console.warn(`⚠️  Missing: ${file}`);
    errors++;
    continue;
  }

  try {
    copyFileSync(srcPath, dstPath);
    console.log(`✓ Copied ${file}`);
    copied++;
  } catch (err) {
    console.error(`✗ Failed to copy ${file}:`, err.message);
    errors++;
  }
}

const typeSrcPath = join(wasmBuildDir, "l2encdec.js.d.ts");
const typeDstPath = join(srcDir, "l2encdec.js.d.ts");
if (existsSync(typeSrcPath)) {
  try {
    copyFileSync(typeSrcPath, typeDstPath);
  } catch (err) {
    console.warn("⚠️  Could not copy types to source:", err.message);
  }
}

if (existsSync(licensePath)) {
  try {
    copyFileSync(licensePath, join(srcDir, "LICENSE"));
    console.log("✓ Copied LICENSE");
  } catch (err) {
    console.warn("⚠️  Could not copy LICENSE:", err.message);
  }
}

if (errors > 0) {
  console.error(
    `\n❌ ${errors} file(s) missing.\nMake sure 'npm run wasm:build' completed successfully.`
  );
  process.exit(1);
}

console.log(`\n✅ WASM artifacts ready (${copied} files)`);
