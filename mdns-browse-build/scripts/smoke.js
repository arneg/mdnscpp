// Minimal load test for the `mdns-browse` package: resolves the native module
// through node-gyp-build exactly like a consumer would and checks the public
// exports. It does not need a running mDNS responder, so it works inside bare
// build containers.

import assert from "node:assert/strict";
import { dirname } from "node:path";
import { fileURLToPath } from "node:url";
import load from "node-gyp-build";

const packageDir = dirname(fileURLToPath(import.meta.resolve("mdns-browse")));

// Which prebuild would be picked (shows the libc/arch selection in CI logs).
console.log("prebuild:", load.path(packageDir));

const mod = await import("mdns-browse");
assert.equal(typeof mod.startBrowse, "function", "startBrowse export");
assert.equal(typeof mod.find, "function", "find export");

console.log(
  `mdns-browse loaded on ${process.platform}-${process.arch}`,
  `(node ${process.version}, node-api ${process.versions.napi})`,
);
