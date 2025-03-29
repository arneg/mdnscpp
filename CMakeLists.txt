cmake_minimum_required(VERSION 3.8)

project(libmdnscpp LANGUAGES CXX)

add_library(libmdnscpp)

target_sources(libmdnscpp PRIVATE
  include/mdnscpp/BrowseResult.h
  include/mdnscpp/Browser.h
  include/mdnscpp/Platform.h
  include/mdnscpp/EventLoop.h
  include/mdnscpp/PollLoop.h
  include/mdnscpp/CallQueue.h
  include/mdnscpp/utils.h
  src/BrowseResult.cpp
  src/Browser.cpp
  src/Platform.cpp
  src/EventLoop.cpp
  src/PollLoop.cpp
  src/CallQueue.cpp
  src/sockAddrToString.h
  src/sockAddrToString.cpp
  src/sockAddrToIPProtocol.h
  src/sockAddrToIPProtocol.cpp
  src/utils.cpp
)

find_library(LIB_DNS_SD NAMES dns_sd)
find_library(LIB_AVAHI NAMES avahi-client)

if(LIB_AVAHI)
  message("Found avahi.")

  target_sources(libmdnscpp PRIVATE
    src/platforms/avahi/AvahiPlatform.h
    src/platforms/avahi/AvahiPlatform.cpp
    src/platforms/avahi/AvahiBrowser.h
    src/platforms/avahi/AvahiBrowser.cpp
  )
  target_compile_definitions(libmdnscpp PRIVATE LIBMDNS_PLATFORM_AVAHI)
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
  target_compile_definitions(libmdnscpp PRIVATE LIBMDNS_PLATFORM_DNSSD)

  if(LIB_DNS_SD)
    target_link_libraries(libmdnscpp PRIVATE "${LIB_DNS_SD}")
  endif(LIB_DNS_SD)
else()
  message(FATAL_ERROR "No platform support found.")
endif()

target_compile_features(libmdnscpp PUBLIC cxx_std_17)
target_include_directories(libmdnscpp PUBLIC include)

add_subdirectory(example)
