# This file was taken from the repository https://github.com/xenoscopic/libuv-cmake . It was made
# available there under the MIT license.
# Changes:
# - Renamed all occurances of LIBUV to libuv to match FindPackageHandleStandardArgs
#   conventions.
# ----
# Standard FIND_PACKAGE module for libuv, sets the following variables:
#   - libuv_FOUND
#   - libuv_INCLUDE_DIRS (only if libuv_FOUND)
#   - libuv_LIBRARIES (only if libuv_FOUND)

# Try to find the header
FIND_PATH(libuv_INCLUDE_DIR NAMES uv.h)

# Try to find the library
FIND_LIBRARY(libuv_LIBRARY NAMES uv libuv)

# Handle the QUIETLY/REQUIRED arguments, set libuv_FOUND if all variables are
# found
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libuv
                                  REQUIRED_VARS
                                  libuv_LIBRARY
                                  libuv_INCLUDE_DIR)

# Hide internal variables
MARK_AS_ADVANCED(libuv_INCLUDE_DIR libuv_LIBRARY)

# Set standard variables
IF(libuv_FOUND)
    SET(libuv_INCLUDE_DIRS "${libuv_INCLUDE_DIR}")
    SET(libuv_LIBRARIES "${libuv_LIBRARY}")
ENDIF()
