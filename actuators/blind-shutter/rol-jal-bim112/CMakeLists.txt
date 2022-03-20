cmake_minimum_required(VERSION 3.5)

# Set toolchain file if not specified
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    get_filename_component(CMAKE_TOOLCHAIN_FILE
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../../CMSIS_CORE_LPC11xx/LPC11xx.cmake
            REALPATH CACHE)
endif()
if(NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(FATAL_ERROR "Toolchain file does not exist: " ${CMAKE_TOOLCHAIN_FILE})
endif()

project(rol_jal_bim112)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "How to build")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

add_definitions(-DBIM112 -DCPP_USE_HEAP)

include_directories(src)
include_directories(${CMAKE_SOURCE_DIR}/../../../common/hand-actuation)
include_directories(${SBLPC_DIR}/inc)
include_directories(${SBLIB_DIR}/inc)

add_executable(rol_jal_bim112.axf
        ../../../common/hand-actuation/hand_actuation.cpp
        src/app-rol-jal.cpp
        src/app-rol-jal.h
        src/app_main.cpp
        src/blind.cpp
        src/blind.h
        src/channel.cpp
        src/channel.h
        src/config.h
        src/cr_cpp_config.cpp
        src/cr_startup_lpc11xx.cpp
        src/crp.c
        src/shutter.cpp
        src/shutter.h)

target_link_libraries(rol_jal_bim112.axf
        -nostdlib
        -L"${SBLPC_DIR}/${BUILD_DIR_PREFIX}"
        -L"${SBLIB_DIR}/${BUILD_DIR_PREFIX}-bim112"
        -Xlinker -Map="rol_jal_bim112.map"
        -Xlinker --gc-sections
        -mcpu=cortex-m0 -mthumb
        -T"${SBLPC_DIR}/LPC1115.ld"
        -lCMSIS_CORE_LPC11xx -lsblib)

add_custom_target(flash
        COMMAND "${SBLPC_DIR}/flash_link2" "${CMAKE_BINARY_DIR}/rol_jal_bim112.axf" "${MCUX_IDE_BIN}"
        DEPENDS rol_jal_bim112.axf)
