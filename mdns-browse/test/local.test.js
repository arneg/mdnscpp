import { afterEach, describe, test } from "node:test";
import { equal } from "node:assert/strict";
import { find } from "mdns-browse";
import { announceService } from "simple-mdns-announce";
import { randomBytes } from "node:crypto";
import { deepEqual } from "node:assert";

async function findService(service) {
  const [type, protocol] = service.type.split(".");
  const result = await find(
    {
      type,
      protocol,
    },
    (result) => {
      return result.name === service.name;
    },
    AbortSignal.timeout(60 * 1000)
  );

  equal(result.type, type);
  equal(result.protocol, protocol);
  equal(result.port, service.port);

  if (service.txtRecords) {
    const tmp = Object.entries(service.txtRecords).map(([key, value]) => {
      return { key, value };
    });
    deepEqual(tmp, result.txtRecords || []);
  }
}

function randomizeServiceName(service) {
  return {
    ...service,
    name: "test-" + randomBytes(8).toString("hex"),
    port: 1 + Math.round(Math.random() * 0xfff1),
  };
}

const type = "_ttest._tcp";

const exampleServices = {
  basic: { type },
  "with txtRecord": { type, txtRecords: { foo: "bar" } },
  "with txtRecords": { type, txtRecords: { foo: "bar", bar: "foo" } },
  "with empty txtRecord": { type, txtRecords: { foo: "bar", bar: "" } },
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
        await findService(service);
      }
    );

    test(
      `simple find before announce (${variant})`,
      { concurrency: true },
      async () => {
        const service = randomizeServiceName(srv);
        const findPromise = await findService(service);
        await new Promise((resolve) => setTimeout(resolve, 4000));
        cleanup = announceService(service);
        await findPromise;
      }
    );
  }
});
