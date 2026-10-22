{
  "targets": [
    {
      "target_name": "mdns-browse-native",
      "sources": [
        "src/mdns-browse.cpp",
        "<!@(node scripts/list-sources.js common)"
      ],
      "include_dirs": [
        "../include/",
        "<!@(node -p \"require('node-addon-api').include\")",
        "<!@(node -p \"require('get-uv-event-loop-napi-h').include\")"
      ],
      "defines": [
        "NAPI_VERSION=9",
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "conditions": [
        [
          "OS==\"mac\"",
          {
            "sources": [
              "<!@(node scripts/list-sources.js mac)"
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
              "<!@(node scripts/list-sources.js linux)"
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
              "<!@(node scripts/list-sources.js win)"
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
