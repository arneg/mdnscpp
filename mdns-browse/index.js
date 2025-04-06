import { default as bindings } from "bindings";

const { startBrowse } = bindings("mdns-browse-native.node");

// Use your bindings defined in your C files

const cleanup = startBrowse(
  {
    type: "_oca",
    protocol: "_tcp",
  },
  (results) => {
    console.log("results", results);
  }
);
