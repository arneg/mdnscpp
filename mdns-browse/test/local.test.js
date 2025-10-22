import { afterEach, describe, test } from "node:test";
import { startBrowse } from "mdns-browse";
import { announceService } from "simple-mdns-announce";
import {
  findAndCompareService,
  combineCleanup,
  randomizeServiceName,
} from "./helpers.js";

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
