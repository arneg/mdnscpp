import { afterEach, describe, test } from "node:test";
import { equal } from "node:assert/strict";
import { find } from "mdns-browse";
import { announceService } from "simple-mdns-announce";

describe("local", () => {
  let cleanup;

  afterEach(() => {
    console.log("after each");
    if (!cleanup) return;
    try {
      cleanup();
    } catch (err) {
      console.error("failed", err);
    }
  });

  test("simple find", { concurrency: false }, async () => {
    const service = {
      name: "testing",
      type: "_ttest._tcp",
      port: 1234,
    };
    cleanup = announceService(service);

    const result = await find(
      {
        type: "_ttest",
        protocol: "_tcp",
      },
      (result) => {
        return result.name === service.name;
      },
      AbortSignal.timeout(30 * 1000)
    );

    equal(result.name, service.name);
    equal(result.port, service.port);
    cleanup();
  });
});
