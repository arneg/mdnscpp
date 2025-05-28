import { afterEach, describe, test } from "node:test";
import { equal } from "node:assert/strict";
import { find, startBrowse } from "mdns-browse";
import { announceService } from "simple-mdns-announce";
import { randomBytes } from "node:crypto";
import { deepEqual } from "node:assert";
import { platform } from "node:os";

function normalizeTxtRecords(txtRecords) {
  return (txtRecords || []).map(({ key, value }) => {
    if (platform() === "win32") {
      // the win32 API cannot differentiate between missing and empty
      // values.
      if (!value) value = "";
    }
    return { key, value };
  });
}

function combineCleanup(...callbacks) {
  return () => {
    callbacks.forEach((cb) => {
      try {
        cb();
      } catch (err) {
        console.error("cleanup threw error", err);
      }
    });
  };
}

async function waitFor(predicate, timeout) {
  const signal = AbortSignal.timeout(timeout);
  let timerId;
  return new Promise((resolve, reject) => {
    timerId = setInterval(() => {
      if (signal.aborted) {
        reject(signal.reason);
      } else if (predicate()) {
        resolve();
      }
    }, 1000);
  }).finally(() => {
    clearInterval(timerId);
  });
}

function browseCriteriaFromService(service) {
  const [type, protocol] = service.type.split(".");

  return { type, protocol };
}

function compareServiceAndResult(service, result) {
  const criteria = browseCriteriaFromService(service);
  equal(result.type, criteria.type);
  equal(result.protocol, criteria.protocol);
  equal(result.port, service.port);

  if (service.txtRecords) {
    const tmp = Object.entries(service.txtRecords).map(([key, value]) => ({
      key,
      value,
    }));

    deepEqual(normalizeTxtRecords(tmp), normalizeTxtRecords(result.txtRecords));
  }
}

async function findAndCompareService(service) {
  const criteria = browseCriteriaFromService(service);
  const result = await find(
    criteria,
    (result) => {
      return result.name === service.name;
    },
    AbortSignal.timeout(60 * 1000)
  );

  compareServiceAndResult(service, result);
}

function randomizeServiceName(service) {
  return {
    ...service,
    name: (service.name || "test") + "-" + randomBytes(8).toString("hex"),
    port: 1 + Math.round(Math.random() * 0xfff1),
  };
}

const type = "_ttest._tcp";

const exampleServices = {
  basic: { type },
  "with txtRecord": { type, txtRecords: { foo: "bar" } },
  "with txtRecords": { type, txtRecords: { foo: "bar", bar: "foo" } },
  "with empty txtRecord": { type, txtRecords: { foo: "bar", bar: "" } },
  "with unicode name": { type, name: "капуста" },
};

describe("local", () => {
  let cleanup;

  afterEach(() => {
    if (!cleanup) return;
    try {
      cleanup();
    } catch (err) {
      console.error("failed", err);
    }
  });

  for (const [variant, srv] of Object.entries(exampleServices)) {
    test(
      `simple find after announce (${variant})`,
      { concurrency: true },
      async () => {
        const service = randomizeServiceName(srv);
        cleanup = announceService(service);
        await new Promise((resolve) => setTimeout(resolve, 4000));
        await findAndCompareService(service);
      }
    );

    test(
      `simple find before announce (${variant})`,
      { concurrency: true },
      async () => {
        const service = randomizeServiceName(srv);
        const findPromise = findAndCompareService(service);
        await new Promise((resolve) => setTimeout(resolve, 4000));
        cleanup = announceService(service);
        await findPromise;
      }
    );

    test(`result disappears and re-appear (${variant})`, async () => {
      const service = randomizeServiceName(srv);
      const criteria = browseCriteriaFromService(service);
      let cleanupAnnounce = announceService(service);
      let results = [];

      const matchesName = ({ name }) => name === service.name;

      cleanup = combineCleanup(
        startBrowse(criteria, (res) => {
          results = res;
        }),
        () => cleanupAnnounce()
      );

      // wait for service to appear
      await waitFor(() => {
        return results.find(matchesName);
      }, 60 * 1000);

      results
        .filter(matchesName)
        .forEach((result) => compareServiceAndResult(service, result));

      cleanupAnnounce();

      // wait for service to disappear
      await waitFor(() => {
        return !results.find(({ name }) => name == service.name);
      }, 2 * 120 * 1000);

      // wait for service to appear again

      cleanupAnnounce = announceService(service);

      // wait for service to appear again
      await waitFor(() => {
        return results.find(matchesName);
      }, 60 * 1000);
    });
  }
});
