#!/usr/bin/env node
// Assemble the mdns-browse npm package. Does not talk to GitHub and never
// publishes.
//
// CI (combine the per-target build artifacts into an unsigned tarball):
//   npm run assemble -- --from-dir dist/from-ci
//
// Maintainer (after a green tag run, on a Mac):
//   1. Download the mdns-browse-package-unsigned artifact from the run page.
//   2. npm run assemble -- --from-tarball <that.tgz-or-zip> --tag mdns-browse-vX.Y.Z \
//        --sign TEAMID
//   3. npm publish the tarball this script prints.

import { spawnSync } from "node:child_process";
import {
  mkdtemp,
  mkdir,
  readFile,
  readdir,
  rm,
  stat,
  writeFile,
} from "node:fs/promises";
import { createRequire } from "node:module";
import { tmpdir } from "node:os";
import { basename, dirname, join, relative, resolve, sep } from "node:path";
import { fileURLToPath } from "node:url";
import { parseArgs } from "node:util";

const require = createRequire(import.meta.url);

const buildDir = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const repoRoot = resolve(buildDir, "..");
const packageDir = resolve(repoRoot, "mdns-browse");
const distDir = resolve(buildDir, "dist");
const prebuildsDir = resolve(packageDir, "prebuilds");

const EXPECTED_KEYS = [
  "darwin-arm64/mdns-browse.node",
  "darwin-x64/mdns-browse.node",
  "linux-arm64/mdns-browse.glibc.node",
  "linux-arm64/mdns-browse.musl.node",
  "linux-x64/mdns-browse.glibc.node",
  "linux-x64/mdns-browse.musl.node",
  "win32-arm64/mdns-browse.node",
  "win32-x64/mdns-browse.node",
];

const EXPECTED_KEY_SET = new Set(EXPECTED_KEYS);
const PLATFORM_DIRS = new Set(EXPECTED_KEYS.map((key) => key.split("/")[0]));

const PACK_WHITELIST =
  /^package\/(LICENSE|README\.md|index\.js|index\.d\.ts|package\.json|prebuilds\/.+\.node)$/;

function usage() {
  return `Usage:
  node scripts/assemble.js --from-dir <dir>
  node scripts/assemble.js --from-tarball <tgz-or-zip> [--sign TEAMID]

Options:
  --from-dir <dir>             CI: directory of per-target .node files.
  --from-tarball <path>        Maintainer: the unsigned npm tarball from CI,
                               or the GitHub artifact zip that contains it.
  --tag <tag>                  Require mdns-browse-vX.Y.Z to match the
                               package.json being packed.
  --sign <TEAMID>              Codesign the darwin binaries with the Developer
                               ID Application identity for this team (macOS
                               only), before packing.
  --skip-smoke                 Do not install-and-import the packed tarball.
  --skip-pack                  Stop after merging or extracting (no npm pack).
`;
}

function fail(message) {
  console.error(`assemble: ${message}`);
  process.exit(1);
}

function run(command, args, options = {}) {
  const result = spawnSync(command, args, {
    encoding: "utf8",
    stdio: ["ignore", "pipe", "pipe"],
    ...options,
  });
  if (result.status !== 0) {
    const detail = (result.stderr || result.stdout || "").trim();
    fail(
      `\`${command} ${args.join(" ")}\` failed` +
        (detail ? `:\n${detail}` : ""),
    );
  }
  return result.stdout;
}

function readPackageJson(dir) {
  return require(join(dir, "package.json"));
}

function isTarballName(name) {
  return name.endsWith(".tgz") || name.endsWith(".tar.gz");
}

function macosSignature(file) {
  if (process.platform !== "darwin") return undefined;
  const result = spawnSync("codesign", ["-dv", file], { encoding: "utf8" });
  const text = `${result.stdout}\n${result.stderr}`;
  if (result.status !== 0) return "unsigned or invalid";
  if (/Signature=adhoc/.test(text)) return "ad-hoc";
  const authority = /Authority=(.+)/.exec(text);
  return authority ? authority[1] : "signed";
}

function resolveDeveloperIdIdentity(teamId) {
  const trimmed = teamId.trim();
  if (!/^[A-Z0-9]{10}$/.test(trimmed)) {
    fail(`--sign expects a 10-character Team ID, got ${JSON.stringify(teamId)}`);
  }
  const listing = run("security", ["find-identity", "-v", "-p", "codesigning"]);
  const matches = [];
  for (const line of listing.split("\n")) {
    const quoted = /"(Developer ID Application: .+)"/.exec(line);
    if (!quoted) continue;
    if (quoted[1].endsWith(`(${trimmed})`)) matches.push(quoted[1]);
  }
  if (matches.length === 0) {
    fail(
      `no Developer ID Application identity for team ${trimmed}. ` +
        `Check: security find-identity -v -p codesigning`,
    );
  }
  if (matches.length > 1) {
    fail(
      `multiple Developer ID Application identities for team ${trimmed}:\n  ${matches.join("\n  ")}`,
    );
  }
  console.log(`signing as ${matches[0]}`);
  return matches[0];
}

function signDarwinBinaries(teamId, root) {
  if (process.platform !== "darwin") {
    fail("--sign only works on macOS");
  }
  const identity = resolveDeveloperIdIdentity(teamId);
  for (const key of EXPECTED_KEYS) {
    if (!key.startsWith("darwin-")) continue;
    const dest = join(root, key);
    console.log(`codesign ${key}`);
    run("codesign", [
      "--force",
      "--timestamp",
      "--options",
      "runtime",
      "--sign",
      identity,
      dest,
    ]);
    run("codesign", ["--verify", "--strict", dest]);
  }
}

async function collectBySuffix(root, suffix) {
  const found = [];
  async function walk(dir) {
    let entries;
    try {
      entries = await readdir(dir, { withFileTypes: true });
    } catch (err) {
      fail(`cannot read ${dir}: ${err.message}`);
    }
    for (const entry of entries) {
      const path = join(dir, entry.name);
      if (entry.isDirectory()) await walk(path);
      else if (entry.name.endsWith(suffix)) found.push(path);
    }
  }
  await walk(root);
  return found;
}

async function collectTarballs(root) {
  const found = [];
  async function walk(dir) {
    const entries = await readdir(dir, { withFileTypes: true });
    for (const entry of entries) {
      const path = join(dir, entry.name);
      if (entry.isDirectory()) await walk(path);
      else if (isTarballName(entry.name)) found.push(path);
    }
  }
  await walk(root);
  return found;
}

function destKeyFor(file, root) {
  const rel = relative(root, file);
  const parts = rel.split(sep);
  const dir = parts.find((part) => PLATFORM_DIRS.has(part));
  if (!dir) {
    fail(
      `${rel} is not under a known prebuild directory ` +
        `(${[...PLATFORM_DIRS].join(", ")})`,
    );
  }
  return `${dir}/${basename(file)}`;
}

function assertPrebuildSet(mapped) {
  const keys = [...mapped.keys()].sort();
  const missing = EXPECTED_KEYS.filter((key) => !mapped.has(key));
  const extra = keys.filter((key) => !EXPECTED_KEY_SET.has(key));
  if (missing.length || extra.length) {
    fail(
      "prebuild set does not match:\n" +
        (missing.length ? `  missing: ${missing.join(", ")}\n` : "") +
        (extra.length ? `  extra: ${extra.join(", ")}` : ""),
    );
  }
}

async function mapNodeFiles(sourceDir) {
  const files = await collectBySuffix(sourceDir, ".node");
  if (files.length === 0) fail(`no .node files under ${sourceDir}`);
  const mapped = new Map();
  for (const file of files) {
    const key = destKeyFor(file, sourceDir);
    if (mapped.has(key)) fail(`duplicate prebuild ${key}`);
    mapped.set(key, file);
  }
  assertPrebuildSet(mapped);
  return mapped;
}

async function reportPrebuilds(root) {
  const report = [];
  for (const key of EXPECTED_KEYS) {
    const dest = join(root, key);
    report.push({
      key,
      size: (await stat(dest)).size,
      signing: key.startsWith("darwin-") ? macosSignature(dest) : undefined,
    });
  }
  return report;
}

async function mergePrebuilds(sourceDir) {
  const mapped = await mapNodeFiles(sourceDir);
  // Read everything first so --from-dir may point at prebuilds/ itself.
  const loaded = [];
  for (const key of EXPECTED_KEYS) {
    loaded.push({ key, buf: await readFile(mapped.get(key)) });
  }

  await rm(prebuildsDir, { recursive: true, force: true });
  await mkdir(prebuildsDir, { recursive: true });

  for (const { key, buf } of loaded) {
    const dest = join(prebuildsDir, key);
    await mkdir(dirname(dest), { recursive: true });
    await writeFile(dest, buf);
  }
  return reportPrebuilds(prebuildsDir);
}

async function locateTarball(source) {
  let info;
  try {
    info = await stat(source);
  } catch (err) {
    fail(`cannot read ${source}: ${err.message}`);
  }

  if (info.isFile()) {
    if (isTarballName(source)) return { tgz: source, cleanup: null };
    if (!source.endsWith(".zip")) {
      fail(`${source} is not a .tgz or .zip`);
    }
    const tmp = await mkdtemp(join(tmpdir(), "mdns-browse-artifact-"));
    console.log(`unzip ${basename(source)}`);
    run("unzip", ["-o", "-q", source, "-d", tmp]);
    return tarballInDir(tmp, tmp);
  }

  return tarballInDir(source, null);
}

async function tarballInDir(dir, cleanup) {
  const tgzs = await collectTarballs(dir);
  if (tgzs.length === 0) {
    fail(
      `no npm tarball under ${dir}. Download mdns-browse-package-unsigned, ` +
        `not the individual prebuild-* artifacts.`,
    );
  }
  if (tgzs.length > 1) {
    fail(
      `multiple tarballs under ${dir}:\n  ${tgzs.map((f) => basename(f)).join("\n  ")}`,
    );
  }
  return { tgz: tgzs[0], cleanup };
}

async function extractNpmTarball(tgz) {
  const tmp = await mkdtemp(join(tmpdir(), "mdns-browse-package-"));
  run("tar", ["-xzf", tgz, "-C", tmp]);
  const dir = join(tmp, "package");
  try {
    await stat(join(dir, "package.json"));
  } catch {
    fail(`${basename(tgz)} does not look like an npm pack tarball`);
  }
  return { dir, cleanup: tmp };
}

function requireTagMatches(version, tag) {
  if (!tag) return;
  if (!/^mdns-browse-v/.test(tag)) {
    fail(`tag ${tag} does not look like mdns-browse-vX.Y.Z`);
  }
  const tagVersion = tag.replace(/^mdns-browse-v/, "");
  if (tagVersion !== version) {
    fail(`tag version ${tagVersion} != package.json version ${version}`);
  }
}

async function packPackage(cwd) {
  await mkdir(distDir, { recursive: true });
  const version = readPackageJson(cwd).version;
  const tarballName = `mdns-browse-${version}.tgz`;
  const tarball = join(distDir, tarballName);
  await rm(tarball, { force: true });
  run("npm", ["pack", "--pack-destination", distDir], { cwd });
  const contents = run("tar", ["-tzf", tarball])
    .split("\n")
    .map((line) => line.trim())
    .filter(Boolean)
    .sort();
  const unexpected = contents.filter((line) => !PACK_WHITELIST.test(line));
  if (unexpected.length) {
    fail(`unexpected files in ${tarballName}:\n  ${unexpected.join("\n  ")}`);
  }
  const packedKeys = contents
    .filter((line) => line.startsWith("package/prebuilds/"))
    .map((line) => line.slice("package/prebuilds/".length));
  const missing = EXPECTED_KEYS.filter((key) => !packedKeys.includes(key));
  if (missing.length) {
    fail(`tarball is missing prebuilds: ${missing.join(", ")}`);
  }
  return tarball;
}

async function smokeTest(tarball) {
  const tmp = await mkdtemp(join(tmpdir(), "mdns-browse-assemble-"));
  try {
    run("npm", ["install", "--ignore-scripts", tarball], { cwd: tmp });
    const script = join(tmp, "smoke.mjs");
    await writeFile(
      script,
      `import { startBrowse, find } from "mdns-browse";
if (typeof startBrowse !== "function" || typeof find !== "function") {
  throw new Error("mdns-browse exports missing");
}
console.log("smoke ok", process.platform, process.arch, process.version);
`,
    );
    run("node", [script], { cwd: tmp });
  } finally {
    await rm(tmp, { recursive: true, force: true });
  }
}

function printReport(report, tarball, signed) {
  console.log("\nprebuilds:");
  for (const row of report) {
    const kb = Math.round(row.size / 1024);
    const extra = row.signing ? `  ${row.signing}` : "";
    console.log(`  ${row.key.padEnd(42)} ${kb}kB${extra}`);
  }
  const adHoc = report.filter((row) => row.signing === "ad-hoc");
  if (adHoc.length && !signed) {
    console.log(
      "\ndarwin binaries are still ad-hoc. Re-run with --sign TEAMID " +
        "before publishing.",
    );
  }
  if (tarball) {
    console.log(`\npackage: ${tarball}`);
    console.log("next:");
    console.log(`  npm publish --dry-run ${tarball}`);
    console.log(`  npm publish ${tarball}`);
  }
}

const { values } = parseArgs({
  options: {
    tag: { type: "string" },
    "from-dir": { type: "string" },
    "from-tarball": { type: "string" },
    sign: { type: "string" },
    "skip-smoke": { type: "boolean", default: false },
    "skip-pack": { type: "boolean", default: false },
    help: { type: "boolean", default: false },
  },
  strict: true,
});

if (values.help) {
  console.log(usage());
  process.exit(0);
}

if (values["from-dir"] && values["from-tarball"]) {
  fail("use either --from-dir or --from-tarball, not both");
}
if (!values["from-dir"] && !values["from-tarball"]) {
  console.error(usage());
  fail("need --from-dir or --from-tarball");
}

const cleanup = [];
try {
  let packRoot;
  let prebuildsRoot;

  if (values["from-dir"]) {
    packRoot = packageDir;
    prebuildsRoot = prebuildsDir;
    requireTagMatches(readPackageJson(packRoot).version, values.tag);
    await mergePrebuilds(resolve(process.cwd(), values["from-dir"]));
  } else {
    const located = await locateTarball(
      resolve(process.cwd(), values["from-tarball"]),
    );
    if (located.cleanup) cleanup.push(located.cleanup);
    const extracted = await extractNpmTarball(located.tgz);
    cleanup.push(extracted.cleanup);
    packRoot = extracted.dir;
    prebuildsRoot = join(packRoot, "prebuilds");
    requireTagMatches(readPackageJson(packRoot).version, values.tag);
    await mapNodeFiles(prebuildsRoot);
  }

  if (values.sign) signDarwinBinaries(values.sign, prebuildsRoot);
  const report = await reportPrebuilds(prebuildsRoot);

  let tarball;
  if (!values["skip-pack"]) {
    tarball = await packPackage(packRoot);
    if (!values["skip-smoke"]) await smokeTest(tarball);
  }

  printReport(report, tarball, Boolean(values.sign));
} finally {
  for (const dir of cleanup) {
    await rm(dir, { recursive: true, force: true });
  }
}
