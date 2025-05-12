# mdnscpp

**mdnscpp** is a lightweight C++ library for browsing mDNS (Multicast DNS) services, also known as DNS-SD (DNS-based Service Discovery). It provides a unified API to discover services across platforms using one of the following backends:

- **Bonjour / dns_sd** (macOS, iOS)
- **Avahi** (Linux)
- **Win32 API** (Windows)

The main goal of this library is to abstract away platform-specific implementations, enabling developers to write portable mDNS client code.

## ✨ Features

- Cross-platform support
- Automatic backend selection
- Asynchronous event-driven design
- Sorted service result listing
- Simple and consistent C++ API

## 🔄 Event Loop Abstraction

`mdnscpp` internally **abstracts the event loop** to allow flexibility and seamless integration with different runtime environments. This makes the library adaptable for a variety of use cases, from simple command-line tools to full-featured GUI or server applications.

### Built-in Event Loop Implementations

- **Default Poll-Based Loop**  
  A minimal, platform-independent implementation using `poll()`—intended primarily for testing and standalone use.

- **libuv Integration**  
  Full support for [libuv](https://libuv.org/), enabling integration with applications already using the `libuv` event loop (e.g. Node.js backends or other asynchronous systems).

### Extensibility

The event loop system is designed with extensibility in mind. Future or user-contributed integrations could include support for other frameworks such as **Boost.Asio**.

This architecture ensures that `mdnscpp` can be embedded cleanly into existing systems without imposing its own threading or timing model.

## 🔧 Requirements

- C++17 or later
- Platform-specific mDNS backend:
  - `dns_sd.h` (Bonjour) on macOS/iOS
  - `avahi-client` and `avahi-common` on Linux
  - Native DNS-SD support on Windows

---

## 🛠️ Building

You can build `mdnscpp` using CMake:

```bash
cmake -Bbuild
cmake --build build -j4
```

## 🚀 Example Usage

```cpp
#include <iostream>

#include <mdnscpp/DefaultLoop.h>
#include <mdnscpp/Platform.h>
#include <mdnscpp/utils.h>

int main(int argc, const char **argv)
{
    mdnscpp::DefaultLoop loop;

    auto platform = mdnscpp::createPlatform(loop);

    auto browser = platform->createBrowser("_http_", "_tcp", [](auto browser) {
        auto results = mdnscpp::getSortedList(browser->getResults());
        std::cout << "Results (" << results.size() << "): " << std::endl;
        for (const auto &result : results)
        {
            std::cout << result.describe() << std::endl;
        }
    });

    loop.run();
    return 0;
}
```

This example sets up a service browser for services advertising \_oca.\_tcp, prints out sorted results, and runs the event loop until interrupted.

## 📦 API Overview

- `mdnscpp::DefaultLoop` - Event loop abstraction
- `mdnscpp::createPlatform()` - Initializes platform-specific backend
- `Platform::createBrowser()` - Starts browsing for a given service type
- `Browser::getResults()` - Access discovered services
- `mdnscpp::getSortedList()` - Helper to get sorted service list
- `ServiceResult::describe()` - Returns a human-readable description of the result

## 🙋‍♂️ Contributions

Contributions, suggestions, and bug reports are welcome! Please open an issue or submit a pull request.

## 📫 Contact

For questions or feedback, feel free to reach out or open an issue on the repository.

## 📜 License

This library is released under the terms of the MIT License.

Copyright (c) 2024-2025 Arne G&ouml;deke
