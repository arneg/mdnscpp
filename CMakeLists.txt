cmake_minimum_required(VERSION 3.8)

project(libmdnscpp LANGUAGES CXX)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FindOrFetchLibuv)

add_library(libmdnscpp)

target_sources(libmdnscpp PRIVATE
  include/mdnscpp/BrowseResult.h
  include/mdnscpp/Browser.h
  include/mdnscpp/Platform.h
  include/mdnscpp/EventLoop.h
  include/mdnscpp/CallQueue.h
  include/mdnscpp/utils.h
  include/mdnscpp/DefaultLoop.h
  include/mdnscpp/IPAddress.h
  src/BrowseResult.cpp
  src/TxtRecord.cpp
  src/Browser.cpp
  src/Platform.cpp
  src/EventLoop.cpp
  src/CallQueue.cpp
  src/IPAddress.cpp
  src/sockAddrToString.h
  src/sockAddrToString.cpp
  src/sockAddrToIPProtocol.h
  src/sockAddrToIPProtocol.cpp
  src/utils.cpp
)

find_library(LIB_DNS_SD NAMES dns_sd)
find_library(LIB_AVAHI NAMES avahi-client)


if(NOT WIN32)
  target_sources(libmdnscpp PRIVATE
    include/mdnscpp/PollLoop.h
    src/PollLoop.cpp
  )
  target_compile_definitions(libmdnscpp PUBLIC LIBMDNS_LOOP_POLL)
endif()

if(libuv_FOUND)
  message("using libuv")
  target_sources(libmdnscpp PRIVATE
    include/mdnscpp/LibuvLoop.h
    src/LibuvLoop.cpp
  )
  target_link_libraries(libmdnscpp PUBLIC "${libuv_LIBRARIES}")
  target_compile_definitions(libmdnscpp PUBLIC LIBMDNS_LOOP_LIBUV)
  target_include_directories(libmdnscpp PUBLIC "${libuv_INCLUDE_DIRS}")
endif()

if(LIB_AVAHI)
  message("Found avahi.")

  target_sources(libmdnscpp PRIVATE
    src/platforms/avahi/AvahiPlatform.h
    src/platforms/avahi/AvahiPlatform.cpp
    src/platforms/avahi/AvahiBrowser.h
    src/platforms/avahi/AvahiBrowser.cpp
  )
  target_compile_definitions(libmdnscpp PUBLIC LIBMDNS_PLATFORM_AVAHI)
  target_link_libraries(libmdnscpp PRIVATE avahi-client)
elseif(LIB_DNS_SD OR APPLE)
  message("Found libdns_sd.")
  target_sources(libmdnscpp PRIVATE
    src/platforms/dns_sd/DnsSdPlatform.h
    src/platforms/dns_sd/DnsSdPlatform.cpp
    src/platforms/dns_sd/DnsSdRef.h
    src/platforms/dns_sd/DnsSdRef.cpp
    src/platforms/dns_sd/DnsSdBrowser.h
    src/platforms/dns_sd/DnsSdBrowser.cpp
    src/platforms/dns_sd/DnsSdResolve.h
    src/platforms/dns_sd/DnsSdResolve.cpp
    src/platforms/dns_sd/DnsSdGetAddrInfo.h
    src/platforms/dns_sd/DnsSdGetAddrInfo.cpp
  )
  target_compile_definitions(libmdnscpp PUBLIC LIBMDNS_PLATFORM_DNSSD)

  if(LIB_DNS_SD)
    target_link_libraries(libmdnscpp PRIVATE "${LIB_DNS_SD}")
  endif(LIB_DNS_SD)
elseif(WIN32)
  message("Detected win32 platform.")
  target_sources(libmdnscpp PRIVATE 
    src/platforms/win32/Win32Browser.cpp
    src/platforms/win32/Win32Browser.h
    src/platforms/win32/Win32Platform.cpp
    src/platforms/win32/Win32Platform.h
    src/platforms/win32/toWideString.cpp
    src/platforms/win32/toWideString.h
    src/platforms/win32/fromWideString.cpp
    src/platforms/win32/fromWideString.h
    src/platforms/win32/dnsTypeToString.cpp
    src/platforms/win32/dnsTypeToString.h
    src/platforms/win32/Win32Resolve.cpp
    src/platforms/win32/Win32Resolve.h
  )
  target_link_libraries(libmdnscpp PUBLIC dnsapi.dll)
  target_compile_definitions(libmdnscpp PUBLIC LIBMDNS_PLATFORM_WIN32)
else()
  message(FATAL_ERROR "No platform support found.")
endif()

target_compile_features(libmdnscpp PUBLIC cxx_std_17)
target_include_directories(libmdnscpp PUBLIC include)

add_subdirectory(example)
