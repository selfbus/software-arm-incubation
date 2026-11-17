cmake_minimum_required(VERSION 3.28)

project(MultiSensorAktor LANGUAGES CXX VERSION 0.16)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Enable verbose output from the build system")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SB_PATH_TO_SBLIB_REPO "../../../software-arm-lib" CACHE PATH "Path to the sblib repository (e.g. /home/user/sblib), absolute or relative to this directory.")
cmake_path(ABSOLUTE_PATH SB_PATH_TO_SBLIB_REPO BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} NORMALIZE OUTPUT_VARIABLE SBLIB_REPO)

set(SBLPC_DIR ${SBLIB_REPO}/vendor/CMSIS_CORE_LPC11xx CACHE PATH "Path to the 'CMSIS_CORE_LPC11xx' directory")
set(SBLIB_PATH ${SBLIB_REPO}/sblib CACHE PATH "Path to the sblib")

# Toolchain file is set in CMakePresets.json or on command line
if(CMAKE_CROSSCOMPILING AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "Cross-compiling but CMAKE_TOOLCHAIN_FILE not set. Set in CMakePresets.json or configure with -DCMAKE_TOOLCHAIN_FILE=<toolchain file>, e.g. cmake/toolchains/arm-none-eabi_mcuxpresso.cmake of the sblib")
endif()
if (NOT EXISTS "${SBLIB_PATH}")
    message(FATAL_ERROR "sblib project not found. Check setting of SBLIB_PATH (${SBLIB_PATH})")
endif()
if (NOT EXISTS "${SBLPC_DIR}")
    message(FATAL_ERROR "CMSIS_CORE_LPC11xx project not found. Check setting of SBLPC_DIR (${SBLPC_DIR})")
endif()
if(NOT CMAKE_CROSSCOMPILING)
    message(FATAL_ERROR "Unknown architecture. Only cross-compiling for ARM is supported.")
endif()

set(TARGET_ARM ${PROJECT_NAME}_arm)

add_subdirectory(${SBLIB_PATH} sblib_arm)
add_subdirectory(${SBLPC_DIR} CMSIS_CORE_LPC11xx_arm)

include(MultiSensorAktor.cmake)

add_executable(${TARGET_ARM} ${MSA_SRC})
target_link_libraries(${TARGET_ARM} PUBLIC CMSIS_CORE_LPC11xx_arm sblib_arm)
target_include_directories(${TARGET_ARM} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${SBLPC_DIR}/inc
    ${SBLIB_PATH}/inc
)
# same definitions as in the MCUXpresso project (.cproject)
target_compile_definitions(${TARGET_ARM} PRIVATE
    __USE_CMSIS=CMSIS_CORE_LPC11xx
    __LPC11XX__
    __NEWLIB__
    CORE_M0
    CPP_USE_HEAP
    ARDUINO
    SS=0
)
target_compile_options(${TARGET_ARM} PRIVATE
    $<$<CONFIG:Debug>: -O0>
    $<$<CONFIG:Release>: -Os>
)

# 0x3000: behind the Selfbus bootloader (default). 0x0: without bootloader, e.g. flashed via SWD on a test bench.
set(MSA_FLASH_START "0x3000" CACHE STRING "Flash address the application is linked to (0x3000 or 0x0)")
set_property(CACHE MSA_FLASH_START PROPERTY STRINGS "0x3000" "0x0")
if(MSA_FLASH_START STREQUAL "0x3000")
    set(MSA_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linkscripts/MultiSensorAktor_Flashstart_0x3000_combined.ld)
elseif(MSA_FLASH_START STREQUAL "0x0")
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/linkscripts/MultiSensorAktor_Flashstart_0x3000_combined.ld MSA_LINKER_SCRIPT_TEXT)
    string(REPLACE "0x3000" "0x0" MSA_LINKER_SCRIPT_TEXT "${MSA_LINKER_SCRIPT_TEXT}")
    set(MSA_LINKER_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/MultiSensorAktor_Flashstart_0x0_combined.ld)
    file(WRITE ${MSA_LINKER_SCRIPT} "${MSA_LINKER_SCRIPT_TEXT}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/linkscripts/MultiSensorAktor_Flashstart_0x3000_combined.ld)
else()
    message(FATAL_ERROR "MSA_FLASH_START must be 0x3000 or 0x0, not ${MSA_FLASH_START}")
endif()

target_link_options(${TARGET_ARM} PRIVATE
    -T ${MSA_LINKER_SCRIPT}
    -Xlinker -Map=$<TARGET_FILE_DIR:${TARGET_ARM}>/${TARGET_ARM}.map
)

add_custom_command(TARGET ${TARGET_ARM} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_ARM}> $<TARGET_FILE_DIR:${TARGET_ARM}>/${TARGET_ARM}.hex
)
add_custom_command(TARGET ${TARGET_ARM} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_ARM}> $<TARGET_FILE_DIR:${TARGET_ARM}>/${TARGET_ARM}.bin
)
