import { after, describe, test } from "node:test";
import { equal } from "node:assert/strict";
//import { startBrowse } from "mdns-browse";
import { find, startBrowse } from "../index.js";
import { announceService } from "simple-mdns-announce";

describe("local", () => {
  let cleanup;

  after(() => {
    if (!cleanup) return;
    cleanup();
  });

  test("simple find", async () => {
    const service = {
      name: "this-service-is-for-testing",
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
      AbortSignal.timeout(2000)
    );

    equal(result.name, service.name);
    equal(result.port, service.port);
  });
});
