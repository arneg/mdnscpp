import { default as bindings } from "bindings";

const { startBrowse } = bindings("mdns-browse-native.node");

function matchAll() {
  return true;
}

export function find(options, predicate, signal) {
  if (!predicate) predicate = matchAll;

  return new Promise((resolve, reject) => {
    signal?.throwIfAborted();

    let cleanup;

    cleanup = startBrowse(options, (results) => {
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
