cmake_minimum_required (VERSION 2.8)
project(ip_api)
SET(SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src")
aux_source_directory( ${SOURCE_ROOT} SOURCES )

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Default build type is: Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                            "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -D_POSIX_C_SOURCE=200809L")
endif()

if(APPLE)
    INCLUDE_DIRECTORIES("/opt/local/include")
    LINK_DIRECTORIES("/opt/local/lib")
endif()

set(NAME ${PROJECT_NAME})

set(REQUIRED_LIBRARIES
    pthread
    fcgi
    GeoIP)

add_executable(${NAME} ${SOURCES})
target_link_libraries(${NAME} ${REQUIRED_LIBRARIES})
