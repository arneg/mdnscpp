// Prints the mdnscpp library sources for one platform, relative to the
// mdns-browse-build directory and separated by spaces, for use from
// binding.gyp:
//
//   "<!@(node scripts/list-sources.js common)"
//   "<!@(node scripts/list-sources.js mac)"   (also: linux, win)
//
// The list is derived from the parent repository's src/ directory so that it
// cannot drift from the library.

import { readdirSync } from "node:fs";
import { dirname, join, posix, relative, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const buildDir = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const srcDir = resolve(buildDir, "..", "src");

// Not part of the node addon: the addon uses the libuv event loop only.
const excludedSources = ["PollLoop.cpp"];

const platformDirs = {
  mac: "dns_sd",
  linux: "avahi",
  win: "win32",
};

function cppFilesIn(dir) {
  return readdirSync(dir, { withFileTypes: true })
    .filter((entry) => entry.isFile() && entry.name.endsWith(".cpp"))
    .map((entry) => join(dir, entry.name))
    .filter((file) => !excludedSources.includes(file.split(/[\\/]/).pop()));
}

function toGypPath(file) {
  // gyp expects forward slashes, even on Windows.
  return relative(buildDir, file).split(/[\\/]/).join(posix.sep);
}

const target = process.argv[2];
let files;

if (target === "common") {
  files = cppFilesIn(srcDir);
} else if (target in platformDirs) {
  files = cppFilesIn(join(srcDir, "platforms", platformDirs[target]));
} else {
  console.error(
    `usage: list-sources.js <common|${Object.keys(platformDirs).join("|")}>`,
  );
  process.exit(1);
}

if (files.length === 0) {
  console.error(`list-sources.js: no sources found for "${target}"`);
  process.exit(1);
}

console.log(files.map(toGypPath).sort().join(" "));
