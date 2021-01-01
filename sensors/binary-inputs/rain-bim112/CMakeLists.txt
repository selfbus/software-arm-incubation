cmake_minimum_required(VERSION 3.5)

# Set toolchain file if not specified
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    get_filename_component(CMAKE_TOOLCHAIN_FILE
            ${CMAKE_SOURCE_DIR}/../../../../CMSIS_CORE_LPC11xx/LPC11xx.cmake
            REALPATH CACHE)
endif()
if(NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(FATAL_ERROR "Toolchain file does not exist: " ${CMAKE_TOOLCHAIN_FILE})
endif()

set(NAME rain-bim112)
project(${NAME})

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "How to build")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

# these 2 pertain to logging
set(LOGGING OFF CACHE BOOL "Whether to through the serial port")
set(LIB_BUILD_DIR ${BUILD_DIR_PREFIX} CACHE STRING
        "The build subfolder where the libraries reside")

add_definitions(-DBIM112 -Wall -Werror)

add_subdirectory(${SBLIB_DIR} sblib)

include_directories(${SBLPC_DIR}/inc)
include_directories(${SBLIB_DIR}/inc)

file(GLOB SRC src/*.cpp src/*.h)
if (LOGGING)
    add_subdirectory(${SBLOG_DIR} sblog)
    include_directories(${SBLOG_DIR})
    file(GLOB LOGSRC ${SBLOG_DIR}/log/*.cpp ${SBLOG_DIR}/log/*.h)
    list(APPEND SRC ${LOGSRC})
    add_definitions(-DLOGGING)
endif()
message("SRC: ${SRC}")
add_executable(${NAME}.axf ${SRC})

target_link_libraries(${NAME}.axf
        -nostdlib
        -L"${SBLPC_DIR}/${LIB_BUILD_DIR}"
        -L"${SBLIB_DIR}/${LIB_BUILD_DIR}-bim112"
        -Xlinker -Map="${NAME}.map"
        -Xlinker --gc-sections
        -mcpu=cortex-m0 -mthumb
        -T"${SBLPC_DIR}/LPC1115.ld"
        -lCMSIS_CORE_LPC11xx -lsblib)

# out knx2header target
# the 2nd and 3rd command is so cmake gets that its been built
add_custom_command(OUTPUT src/params.h
        COMMAND knx2header -c ${CMAKE_SOURCE_DIR}/resources/conf.py
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/src
        COMMAND cp ${CMAKE_SOURCE_DIR}/src/params.h ${CMAKE_BINARY_DIR}/src/params.h
        DEPENDS conf.py)

# this force builds params.h and a debug version
add_custom_target(knxheader-dbg
        COMMAND knx2header -c ${CMAKE_SOURCE_DIR}/resources/conf.py
        COMMAND knx2header -d -o ${CMAKE_SOURCE_DIR}/test.h
            -t ${CMAKE_SOURCE_DIR}/test.cpp -c ${CMAKE_SOURCE_DIR}/resources/conf.py
        )

add_custom_target(knxclean
        COMMAND rm -f ${CMAKE_SOURCE_DIR}/test.*
        ${CMAKE_SOURCE_DIR}/src/params.*
        ${CMAKE_BINARY_DIR}/src/params.*
        ${CMAKE_SOURCE_DIR}/*.pyc)

add_custom_target(flash
        COMMAND "${SBLPC_DIR}/flash_link2" "${CMAKE_BINARY_DIR}/${NAME}.axf" "${MCUX_IDE_BIN}"
        DEPENDS ${NAME}.axf)

add_custom_target(debug
        COMMAND "${SBLPC_DIR}/debug_link2" "${MCUX_IDE_BIN}"
        DEPENDS ${NAME}.axf)

# redlinkserv -telnetport=3025 3050 3075

