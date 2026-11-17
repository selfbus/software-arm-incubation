cmake_minimum_required(VERSION 3.28)

# Unit tests of the MultiSensorAktor on the host (x64), with the CPU emulation and Catch of the sblib repository.
# The emulated registers are plain memory: the tests drive timers and GPIO by hand, see README.md.
project(MultiSensorAktor-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SB_PATH_TO_SBLIB_REPO "../../../../software-arm-lib" CACHE PATH "Path to the sblib repository (e.g. /home/user/sblib), absolute or relative to this directory.")
cmake_path(ABSOLUTE_PATH SB_PATH_TO_SBLIB_REPO BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} NORMALIZE OUTPUT_VARIABLE SBLIB_REPO)

set(SBLIB_ARCHITECTURE "x64" CACHE STRING "Architecture of the host build (only x64)")
set(CATCH_PATH ${SBLIB_REPO}/Catch CACHE PATH "Path to the Catch project")
set(SBLIB_PATH ${SBLIB_REPO}/sblib CACHE PATH "Path to the sblib")
set(SBLIB_TEST_PATH ${SBLIB_REPO}/test/sblib CACHE PATH "Path to the sblib-test project (CPU emulation)")
if (NOT EXISTS "${SBLIB_TEST_PATH}")
    message(FATAL_ERROR "sblib-test project not found. Check setting of SB_PATH_TO_SBLIB_REPO (${SBLIB_REPO})")
endif()
if(NOT SBLIB_ARCHITECTURE STREQUAL "x64")
    message(FATAL_ERROR "Only SBLIB_ARCHITECTURE x64 is supported, not ${SBLIB_ARCHITECTURE}")
endif()

include(CTest)

# Only what the test target needs (sblib_x64, sblib-test_x64) is built
add_subdirectory(${SBLIB_TEST_PATH} sblib-test_x64 EXCLUDE_FROM_ALL)

set(MSA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(TARGET_X64 ${PROJECT_NAME}_x64)
add_executable(${TARGET_X64}
    src/main.cpp
    src/test_output_pin.cpp
    src/test_pwm_controller.cpp
    src/test_pwm_pin.cpp
    ${MSA_DIR}/src/GenericPin.cpp
    ${MSA_DIR}/src/HelperFunctions.cpp
    ${MSA_DIR}/src/OutputPin.cpp
    ${MSA_DIR}/src/PWMPin.cpp
    ${MSA_DIR}/src/PwmController.cpp
)
target_include_directories(${TARGET_X64} PRIVATE
    ${MSA_DIR}/inc
    ${CATCH_PATH}/inc
    ${SBLIB_PATH}/inc
    ${SBLIB_TEST_PATH}/inc
    ${SBLIB_TEST_PATH}/cpu-emu
)
target_compile_definitions(${TARGET_X64} PRIVATE
    IAP_EMULATION
    __LPC11XX__
)
target_compile_options(${TARGET_X64} PRIVATE -m64 -Wall -Wextra -Wno-unused-parameter -g3 $<$<CONFIG:Debug>:-O0>)
target_link_options(${TARGET_X64} PRIVATE -m64)
# The emulated registers (sblib-test) and the sblib refer to each other
target_link_libraries(${TARGET_X64} "$<LINK_GROUP:RESCAN,sblib-test_x64,sblib_x64>")

add_test(NAME ${TARGET_X64} COMMAND ${TARGET_X64})
