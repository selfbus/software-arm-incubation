cmake_minimum_required(VERSION 3.28)
project(HoermannGateway LANGUAGES CXX VERSION 0.01)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Enable verbose output from the build system")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Creates compile_commands.json file for easier debugging and IDE support

set(SBLPC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../software-arm-lib/vendor/CMSIS_CORE_LPC11xx" CACHE PATH "Path to the 'CMSIS_CORE_LPC11xx' directory")
set(SBLIB_PATH "${CMAKE_SOURCE_DIR}/../../../software-arm-lib/sblib" CACHE PATH "Path to the sblib project")

# Toolchain file is set in CMakePresets.json or on command line
if(CMAKE_CROSSCOMPILING AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "Cross-compiling but CMAKE_TOOLCHAIN_FILE not set. Set in CMakePresets.json or configure with -DCMAKE_TOOLCHAIN_FILE=<path to>/arm-none-eabi_mcuxpresso.cmake")
endif()
if(CMAKE_CROSSCOMPILING AND DEFINED CMAKE_TOOLCHAIN_FILE AND NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    message(FATAL_ERROR "Toolchain file does not exist: ${CMAKE_TOOLCHAIN_FILE}")
endif()
if (NOT EXISTS "${SBLPC_DIR}")
    message(FATAL_ERROR "CMSIS_CORE_LPC11xx project not found. Check setting of SBLPC_DIR (${SBLPC_DIR})")
endif()
if (NOT EXISTS "${SBLIB_PATH}")
    message(FATAL_ERROR "sblib project not found. Check setting of SBLIB_PATH (${SBLIB_PATH})")
endif()


# include source code
include(hoermann_gateway.cmake)

set(TARGET_ARM ${PROJECT_NAME}_arm)

add_executable(${TARGET_ARM} ${SB_HOERMANN_GATEWAY_SRC})

target_include_directories(${TARGET_ARM} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

add_subdirectory(${SBLIB_PATH} sblib)
add_subdirectory(${SBLPC_DIR} CMSIS_CORE_LPC11xx)
target_link_libraries(${TARGET_ARM} PUBLIC sblib_arm CMSIS_CORE_LPC11xx_arm)
add_compile_definitions(
    __LPC11XX__
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Debug>:DEBUG>
)

if(CMAKE_CROSSCOMPILING)
    message(NOTICE "Configuring ${PROJECT_NAME} for ARM architecture.")
    #target_link_options(${TARGET_ARM} PRIVATE -T ${SBLPC_DIR}/LPC1115.ld)
    target_link_options(${TARGET_ARM} PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/linkscripts/HoermannGateway_Debug_complete.ld)
    target_link_options(${TARGET_ARM} PRIVATE -Xlinker -Map=$<TARGET_FILE_DIR:${TARGET_ARM}>/${TARGET_ARM}.map)

    add_custom_command(TARGET ${TARGET_ARM} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_ARM}> $<TARGET_FILE_DIR:${TARGET_ARM}>/${TARGET_ARM}.hex
    )
    add_custom_command(TARGET ${TARGET_ARM} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_ARM}> $<TARGET_FILE_DIR:${TARGET_ARM}>/${TARGET_ARM}.bin
    )
else()
    message(FATAL_ERROR "Unknown architecture. (project ${PROJECT_NAME})")
endif()