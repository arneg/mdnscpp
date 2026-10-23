import load from "node-gyp-build";

function loadNative() {
  try {
    return load(import.meta.dirname);
  } catch (err) {
    const target = `${process.platform}-${process.arch}`;
    const message = String(err?.message ?? err);
    let hint;

    if (/libavahi/i.test(message)) {
      hint =
        "The Avahi client library is required on Linux. Install it, e.g. " +
        "`apt install libavahi-client3` (Debian/Ubuntu), " +
        "`dnf install avahi-libs` (Fedora/RHEL) or " +
        "`apk add avahi-libs` (Alpine).";
    } else if (/No native build was found/i.test(message)) {
      hint =
        `There is no prebuilt binary for ${target}. Supported targets are ` +
        "macOS (arm64, x64), Windows (x64, arm64) and Linux with glibc or " +
        "musl (x64, arm64).";
    }

    throw new Error(
      `mdns-browse: failed to load the native module for ${target}.` +
        (hint ? ` ${hint}` : ""),
      { cause: err },
    );
  }
}

const { startBrowse } = loadNative();

function matchAll() {
  return true;
}

export function find(options, predicate, signal) {
  if (!predicate) predicate = matchAll;

  return new Promise((resolve, reject) => {
    signal?.throwIfAborted();

    let cleanup;

    cleanup = startBrowse(options, (results, error) => {
      if (error) {
        reject(error);
        if (cleanup) cleanup();
        cleanup = null;
        return;
      }

      const result = results.find(predicate);

      if (result) {
        resolve(result);
        if (cleanup) cleanup();
        cleanup = null;
      }
    });

    signal?.addEventListener("abort", () => {
      reject(signal.reason);
      if (cleanup) cleanup();
      cleanup = null;
    });
  });
}

export { startBrowse };
