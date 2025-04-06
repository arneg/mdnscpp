import { join, dirname, resolve, relative } from "node:path";
import {
  copyFile,
  mkdir,
  readdir,
  readFile,
  writeFile,
} from "node:fs/promises";

async function findFiles(...dirs) {
  const result = [];

  for (const dir of dirs) {
    for (const dirent of await readdir(dir, { withFileTypes: true })) {
      if (dirent.isDirectory()) {
        result.push(...(await findFiles(resolve(dir, dirent.name))));
      } else {
        result.push(resolve(dir, dirent.name));
      }
    }
  }

  return result;
}

const files = (await findFiles(join("..", "src"), join("..", "include"))).map(
  (entry) => {
    return relative(resolve(".."), entry);
  }
);

const ignore = ["PollLoop", "DefaultLoop"];

async function importFile(fname) {
  if (ignore.some((entry) => fname.includes(entry))) return;
  const src = join("..", fname);
  const dst = join("src", "mdnscpp", fname);

  await mkdir(dirname(dst), { recursive: true });
  await copyFile(src, dst);
  return dst;
}

const allSources = [];

for (const file of files) {
  const dst = await importFile(file);
  if (dst) allSources.push(dst);
}

function cppOnly(files) {
  return files.filter((file) => file.endsWith(".cpp"));
}

const win32Sources = allSources.filter((file) => file.includes("win32"));
const macosSources = allSources.filter((file) => file.includes("dns_sd"));
const linuxSources = allSources.filter((file) => file.includes("avahi"));
const commonSources = allSources.filter((file) => !file.includes("platform"));

const binding = JSON.parse(await readFile("binding.gyp", { encoding: "utf8" }));

binding.targets[0].sources = ["src/mdns-browse.cpp", ...cppOnly(commonSources)];

binding.targets[0].conditions.forEach(([condition, settings]) => {
  if (condition.includes('OS=="mac"')) settings.sources = cppOnly(macosSources);
  if (condition.includes('OS=="linux"'))
    settings.sources = cppOnly(linuxSources);
  if (condition.includes('OS=="win"')) settings.sources = cppOnly(win32Sources);
});

await writeFile("binding.gyp", JSON.stringify(binding, undefined, 2), {
  encoding: "utf8",
});
