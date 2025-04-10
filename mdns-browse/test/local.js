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
      name: "testing",
      type: "_ttest._tcp",
      port: 1234,
    };
    cleanup = announceService(service);

    await new Promise((resolve) => setTimeout(resolve, 2000));

    const result = await find(
      {
        type: "_ttest",
        protocol: "_tcp",
      },
      (result) => {
        console.log("result: ", result);
        return result.name === service.name;
      },
      AbortSignal.timeout(100000)
    );

    equal(result.name, service.name);
    equal(result.port, service.port);
  });
});
