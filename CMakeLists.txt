cmake_minimum_required (VERSION 3.2)
project(ip_api)
SET(SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src")
aux_source_directory( ${SOURCE_ROOT} SOURCES )

option(WITH_IPV6 "Support for ipv6 addresses" ON)
option(WITH_CHROOT "Support for chroot call" ON)
set(WORKERS 4 CACHE STRING "Workers count")
set_property(CACHE WORKERS PROPERTY STRINGS 2 4 6 7 8 9 10 11 12 13 14 16)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Default build type is: Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                            "MinSizeRel" "RelWithDebInfo")
endif()

configure_file(
    "${SOURCE_ROOT}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
    )

include_directories("${PROJECT_BINARY_DIR}")

if(NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -fPIE -pie \
    -D_POSIX_C_SOURCE=200809L -D_FORTIFY_SOURCE=1 -fstack-protector")

    if(WITH_CHROOT)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEFAULT_SOURCE")
    endif()
endif()

if(APPLE)
    include_directories("/opt/local/include")
    link_directories("/opt/local/lib")
endif()

set(NAME ${PROJECT_NAME})

set(REQUIRED_LIBRARIES
    pthread
    fcgi
    GeoIP)

add_executable(${NAME} ${SOURCES})
target_link_libraries(${NAME} ${REQUIRED_LIBRARIES})
