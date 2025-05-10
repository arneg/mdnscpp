# mdns-browse

**mdns-browse** is a Node.js native addon that provides cross-platform mDNS (DNS-SD) service discovery by integrating using the [mdnscpp](https://github.com/arneg/mdnscpp) C++ library.

This module exposes a simple and consistent JavaScript interface for browsing local network services using the native DNS-SD APIs on each platform (Bonjour, Avahi, or Windows).

## 📦 Installation

```bash
npm install mdns-browse
```

Note: Building this module requires a C++17-capable compiler. On linux you may need to have Bonjour or Avahi development headers installed.

## 🚀 Usage

```ts
import { startBrowse } from "mdns-browse";

const criteria = {
  type: "_http",
  protocol: "_tcp",
  ipProtocol: "ipv4",
};

const browser = startBrowse(criteria, (services) => {
  console.log("Discovered services:", services);
});
```

## 📜 License

This library is released under the terms of the MIT License.

Copyright (c) 2024-2025 Arne G&ouml;deke
