import { describe, it } from "node:test";
import assert from "node:assert/strict";
import { find, startBrowse } from "mdns-browse";

// These tests only exercise argument handling. Every invalid call must throw
// before any platform resources are created, and must not need a working
// mDNS stack.

const valid = { type: "_ttest", protocol: "_tcp" };
const noop = () => {};

function rejects(options, expected) {
  assert.throws(() => startBrowse(options, noop), expected);
}

describe("startBrowse argument validation", () => {
  it("accepts valid options and returns a cleanup function", () => {
    const cleanup = startBrowse(valid, noop);
    assert.equal(typeof cleanup, "function");
    cleanup();
  });

  it("treats null and undefined optional options as not set", () => {
    const cleanup = startBrowse(
      {
        ...valid,
        domain: null,
        interfaceIndex: undefined,
        ipProtocol: null,
      },
      noop
    );
    cleanup();
  });

  describe("call signature", () => {
    it("rejects missing arguments", () => {
      assert.throws(() => startBrowse(), {
        name: "TypeError",
        code: "ERR_MISSING_ARGS",
      });
      assert.throws(() => startBrowse(valid), {
        name: "TypeError",
        code: "ERR_MISSING_ARGS",
      });
    });

    it("rejects a non-object options argument", () => {
      assert.throws(() => startBrowse("_http", noop), {
        name: "TypeError",
        code: "ERR_INVALID_ARG_TYPE",
      });
    });

    it("rejects a non-function callback", () => {
      assert.throws(() => startBrowse(valid, "callback"), {
        name: "TypeError",
        code: "ERR_INVALID_ARG_TYPE",
      });
    });
  });

  describe("types of options", () => {
    for (const [name, options] of Object.entries({
      "missing type": { protocol: "_tcp" },
      "missing protocol": { type: "_ttest" },
      "numeric type": { ...valid, type: 5 },
      "numeric protocol": { ...valid, protocol: 5 },
      "numeric domain": { ...valid, domain: 5 },
      "string interfaceIndex": { ...valid, interfaceIndex: "1" },
      "numeric ipProtocol": { ...valid, ipProtocol: 4 },
    })) {
      it(`rejects ${name} with a TypeError`, () => {
        rejects(options, { name: "TypeError", code: "ERR_INVALID_ARG_TYPE" });
      });
    }
  });

  describe("values of options", () => {
    for (const [name, options, message] of [
      ["type without underscore", { ...valid, type: "http" }, /type/],
      ["type that is too long", { ...valid, type: "_" + "a".repeat(16) }, /type/],
      ["empty type", { ...valid, type: "" }, /type/],
      ["type with invalid characters", { ...valid, type: "_a b" }, /type/],
      ["type with subtype", { ...valid, type: "_a._sub._b" }, /type/],
      ["protocol without underscore", { ...valid, protocol: "tcp" }, /protocol/],
      ["unknown protocol", { ...valid, protocol: "_sctp" }, /protocol/],
      ["empty protocol", { ...valid, protocol: "" }, /protocol/],
      ["domain with empty label", { ...valid, domain: "a..b" }, /domain/],
      [
        "domain with a too long label",
        { ...valid, domain: "a".repeat(64) },
        /domain/,
      ],
      ["domain with control character", { ...valid, domain: "a\nb" }, /domain/],
      ["domain with NUL", { ...valid, domain: "a\0b" }, /domain/],
      ["unknown ipProtocol", { ...valid, ipProtocol: "ipv5" }, /ipProtocol/],
    ]) {
      it(`rejects ${name} with a TypeError`, () => {
        rejects(options, {
          name: "TypeError",
          code: "ERR_INVALID_ARG_VALUE",
          message,
        });
      });
    }

    it("accepts a domain with a trailing dot and non-ASCII characters", () => {
      startBrowse({ ...valid, domain: "local." }, noop)();
      startBrowse({ ...valid, domain: "café.example" }, noop)();
    });
  });

  describe("interfaceIndex", () => {
    for (const interfaceIndex of [-1, 1.5, NaN, Infinity, 2 ** 31, 2 ** 32, 2 ** 53]) {
      it(`rejects ${interfaceIndex} with a RangeError`, () => {
        rejects(
          { ...valid, interfaceIndex },
          { name: "RangeError", code: "ERR_OUT_OF_RANGE" }
        );
      });
    }

    it("accepts 0 (all interfaces)", () => {
      startBrowse({ ...valid, interfaceIndex: 0 }, noop)();
    });
  });
});

describe("find argument validation", () => {
  it("rejects the promise for invalid options", async () => {
    await assert.rejects(find({ type: "http", protocol: "_tcp" }), {
      name: "TypeError",
      code: "ERR_INVALID_ARG_VALUE",
    });
  });
});
