
find_package(libuv)

if (NOT libuv_FOUND)
    include(FetchContent)
    message(STATUS "libuv not found on the system. Downloading and extracting libuv Sources. This will take some time...")
    option(LIBUV_BUILD_SHARED "" OFF)
    FetchContent_Declare(
        libuv
        GIT_REPOSITORY https://github.com/libuv/libuv.git
        GIT_TAG "v1.x"
        GIT_SHALLOW 1
    )
    FetchContent_GetProperties(libuv)
    FetchContent_MakeAvailable(libuv)
        
    add_library(uv ALIAS uv_a)
    SET(libuv_INCLUDE_DIRS "")
    SET(libuv_LIBRARIES "uv")
    SET(libuv_FOUND true)
endif()

return(PROPAGATE libuv_FOUND libuv_INCLUDE_DIRS libuv_LIBRARIES)
