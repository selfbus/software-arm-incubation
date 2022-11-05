cmake_minimum_required(VERSION 3.5)

get_filename_component(TOPDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../.. REALPATH CACHE)
message("top: ${TOPDIR}")
set(LPC_DIR ${TOPDIR}/CMSIS_CORE_LPC11xx CACHE STRING "Where the LPC11xx lib is")
set(SBLIB_DIR ${TOPDIR}/software-arm-lib/sblib CACHE STRING "Where the selfbus lib is")

# Set toolchain file if not specified
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${TOPDIR}/CMSIS_CORE_LPC11xx/LPC11xx.cmake)
endif()
if(NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(FATAL_ERROR "Toolchain file does not exist: ${CMAKE_TOOLCHAIN_FILE}")
endif()
message(STATUS "Toolchain file: " ${CMAKE_TOOLCHAIN_FILE})

project(in8_bcu1)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "How to build")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
set(BusType "BCU_TYPE=10")

add_definitions(-D${BusType})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(DEP_PREFIX cmake-build-release)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEP_PREFIX cmake-build-debug)
endif()

add_subdirectory(${SBLIB_DIR} sblib)

include_directories(${LPC_DIR}/inc)
include_directories(${SBLIB_DIR}/inc)
include_directories(src)

file(GLOB SRC src/*.cpp src/*.c)

add_executable(in8_bcu1.afx ${SRC})

target_link_libraries(in8_bcu1.afx
        -L"${LPC_DIR}/${DEP_PREFIX}"
        -L"${SBLIB_DIR}/${DEP_PREFIX}-bcu1"
        -Xlinker -Map="in8_bcu1.map"
        -Xlinker --gc-sections
        --specs=nosys.specs
        -mcpu=cortex-m0 -mthumb
        -lCMSIS_CORE_LPC11xx -lsblib)

