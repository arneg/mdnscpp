{
  "targets": [
    {
      "target_name": "mdns-browse-native",
      "sources": [
        "src/mdns-browse.cpp",
        "src/mdnscpp/src/BrowseResult.cpp",
        "src/mdnscpp/src/Browser.cpp",
        "src/mdnscpp/src/CallQueue.cpp",
        "src/mdnscpp/src/EventLoop.cpp",
        "src/mdnscpp/src/IPAddress.cpp",
        "src/mdnscpp/src/LibuvLoop.cpp",
        "src/mdnscpp/src/Platform.cpp",
        "src/mdnscpp/src/TxtRecord.cpp",
        "src/mdnscpp/src/sockAddrToIPProtocol.cpp",
        "src/mdnscpp/src/sockAddrToString.cpp",
        "src/mdnscpp/src/utils.cpp"
      ],
      "include_dirs": [
        "src/mdnscpp/include/",
        "<!@(node -p \"require('node-addon-api').include\")",
        "<!@(node -p \"require('get-uv-event-loop-napi-h').include\")"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS",
        "MDNSCPP_ENABLE_VERBOSE_DEBUG"
      ],
      "conditions": [
        [
          "OS==\"mac\"",
          {
            "sources": [
              "src/mdnscpp/src/platforms/dns_sd/DnsSdBrowser.cpp",
              "src/mdnscpp/src/platforms/dns_sd/DnsSdGetAddrInfo.cpp",
              "src/mdnscpp/src/platforms/dns_sd/DnsSdPlatform.cpp",
              "src/mdnscpp/src/platforms/dns_sd/DnsSdRef.cpp",
              "src/mdnscpp/src/platforms/dns_sd/DnsSdResolve.cpp"
            ],
            "defines": [
              "LIBMDNS_PLATFORM_DNSSD"
            ],
            "libraries": []
          }
        ],
        [
          "OS==\"linux\"",
          {
            "sources": [
              "src/mdnscpp/src/platforms/avahi/AvahiBrowser.cpp",
              "src/mdnscpp/src/platforms/avahi/AvahiPlatform.cpp",
              "src/mdnscpp/src/platforms/avahi/AvahiResolver.cpp",
              "src/mdnscpp/src/platforms/avahi/AvahiUtils.cpp"
            ],
            "defines": [
              "LIBMDNS_PLATFORM_AVAHI"
            ],
            "libraries": [
              "-lavahi-client",
              "-lavahi-common"
            ]
          }
        ],
        [
          "OS==\"win\"",
          {
            "sources": [
              "src/mdnscpp/src/platforms/win32/Win32Browser.cpp",
              "src/mdnscpp/src/platforms/win32/Win32Platform.cpp",
              "src/mdnscpp/src/platforms/win32/Win32Resolve.cpp",
              "src/mdnscpp/src/platforms/win32/dnsTypeToString.cpp",
              "src/mdnscpp/src/platforms/win32/fromWideString.cpp",
              "src/mdnscpp/src/platforms/win32/toWideString.cpp"
            ],
            "defines": [
              "LIBMDNS_PLATFORM_WIN32",
              "UNICODE",
              "_UNICODE"
            ],
            "libraries": [
              "dnsapi.lib",
              "Ws2_32.lib"
            ]
          }
        ]
      ]
    }
  ]
}