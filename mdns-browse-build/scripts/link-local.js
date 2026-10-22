// Copies the locally built prebuilds (./prebuilds, produced by
// `npm run prebuildify`) into ../mdns-browse/prebuilds so that the tests
// exercise the real package layout through node-gyp-build.

import { cp, mkdir, readdir, rm } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const buildDir = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const source = resolve(buildDir, "prebuilds");
const target = resolve(buildDir, "..", "mdns-browse", "prebuilds");

let entries;
try {
  entries = await readdir(source);
} catch {
  entries = [];
}

if (entries.length === 0) {
  console.error(`No prebuilds in ${source}. Run "npm run prebuildify" first.`);
  process.exit(1);
}

await rm(target, { recursive: true, force: true });
await mkdir(target, { recursive: true });
await cp(source, target, { recursive: true });

console.log(`Copied ${entries.join(", ")} to ${target}`);
