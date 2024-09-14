cmake_minimum_required(VERSION 3.8)

project(libmdnscpp LANGUAGES CXX)

add_library(libmdnscpp)

target_sources(libmdnscpp PRIVATE
  include/mdnscpp/BrowseResult.h
  include/mdnscpp/Browser.h
  include/mdnscpp/Platform.h
  include/mdnscpp/EventLoop.h
  include/mdnscpp/PollLoop.h
  src/BrowseResult.cpp
  src/Browser.cpp
  src/Platform.cpp
  src/EventLoop.cpp
  src/PollLoop.cpp
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
elseif(LIB_DNS_SD)
  message("Found libdns_sd.")
  target_sources(libmdnscpp PRIVATE
    src/platforms/DnsSd/DnsSdPlatform.h
    src/platforms/DnsSd/DnsSdPlatform.cpp
    src/platforms/DnsSd/DnsSdRef.h
    src/platforms/DnsSd/DnsSdRef.cpp
    src/platforms/DnsSd/DnsSdBrowser.h
    src/platforms/DnsSd/DnsSdBrowser.cpp
    src/platforms/DnsSd/DnsSdResolve.h
    src/platforms/DnsSd/DnsSdResolve.cpp
  )
  target_compile_definitions(libmdnscpp PRIVATE LIBMDNS_PLATFORM_DNSSD)
  target_link_libraries(libmdnscpp PRIVATE "${LIB_DNS_SD}")
elseif(APPLE)
  message("Using apple dns_sd.")
  target_sources(libmdnscpp PRIVATE
    src/platforms/DnsSd/DnsSdPlatform.h
    src/platforms/DnsSd/DnsSdPlatform.cpp
    src/platforms/DnsSd/DnsSdRef.h
    src/platforms/DnsSd/DnsSdRef.cpp
    src/platforms/DnsSd/DnsSdBrowser.h
    src/platforms/DnsSd/DnsSdBrowser.cpp
    src/platforms/DnsSd/DnsSdResolve.h
    src/platforms/DnsSd/DnsSdResolve.cpp
  )
  target_compile_definitions(libmdnscpp PRIVATE LIBMDNS_PLATFORM_DNSSD)
else()
  message(FATAL_ERROR "No platform support found.")
endif()

target_compile_features(libmdnscpp PUBLIC cxx_std_17)
target_include_directories(libmdnscpp PUBLIC include)

add_subdirectory(example)
