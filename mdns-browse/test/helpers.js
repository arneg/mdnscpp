import { find } from "mdns-browse";
import { equal } from "node:assert/strict";
import { randomBytes } from "node:crypto";
import { deepEqual } from "node:assert";
import { platform } from "node:os";

export async function findAndCompareService(service, extraCriteria) {
  const criteria = browseCriteriaFromService(service, extraCriteria);
  const result = await find(
    criteria,
    (result) => {
      return result.name === service.name;
    },
    AbortSignal.timeout(60 * 1000)
  );

  compareServiceAndResult(service, result);
  return result;
}

export function randomizeServiceName(service) {
  return {
    ...service,
    name: (service.name || "test") + "-" + randomBytes(8).toString("hex"),
    port: 1 + Math.round(Math.random() * 0xfff1),
  };
}

export function normalizeTxtRecords(txtRecords) {
  return (txtRecords || []).map(({ key, value }) => {
    if (platform() === "win32") {
      // the win32 API cannot differentiate between missing and empty
      // values.
      if (!value) value = "";
    }
    return { key, value };
  });
}

export function combineCleanup(...callbacks) {
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

export async function waitFor(predicate, timeout) {
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

export function browseCriteriaFromService(service, extraCriteria) {
  const [type, protocol] = service.type.split(".");

  return { type, protocol, ...(extraCriteria || {}) };
}

export function compareServiceAndResult(service, result) {
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
