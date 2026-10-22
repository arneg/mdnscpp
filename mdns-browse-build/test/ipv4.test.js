import { afterEach, describe, test } from "node:test";
import { announceService } from "simple-mdns-announce";
import { findAndCompareService, randomizeServiceName } from "./helpers.js";
import assert, { equal } from "assert";
import { isIPv4 } from "node:net";

const type = "_ttest._tcp";

const exampleServices = {
  basic: { type },
  "with txtRecord": { type, txtRecords: { foo: "bar" } },
  "with txtRecords": { type, txtRecords: { foo: "bar", bar: "foo" } },
  "with empty txtRecord": { type, txtRecords: { foo: "bar", bar: "" } },
  "with unicode name": { type, name: "капуста" },
};

describe("local ipv4", () => {
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
        const discoveredService = await findAndCompareService(service, {
          ipProtocol: "ipv4",
        });
        assert(isIPv4(discoveredService.address));
      }
    );
  }
});
