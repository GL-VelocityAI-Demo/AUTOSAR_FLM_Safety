##############################################################################
# CMakeLists.txt
# AUTOSAR FLM Safety Use Case Build Configuration
#
# AUTOSAR Classic Platform R23-11
##############################################################################

cmake_minimum_required(VERSION 3.16)
project(AUTOSAR_FLM_SafetyUseCase
    VERSION 1.0.0
    DESCRIPTION "AUTOSAR Front Light Management Safety Use Case"
    LANGUAGES CXX
)

##############################################################################
# Build Options
##############################################################################

option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_WARNINGS "Enable compiler warnings" ON)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

##############################################################################
# C++ Standard Configuration
##############################################################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

##############################################################################
# Compiler Flags
##############################################################################

if(ENABLE_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wcast-qual
            -Wformat=2
            -Wundef
            -Werror=return-type
            -Wno-unused-parameter
        )
    elseif(MSVC)
        add_compile_options(/W4 /WX)
    endif()
endif()

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
endif()

##############################################################################
# Include Directories
##############################################################################

set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/src
)

##############################################################################
# Source Files
##############################################################################

set(APPLICATION_SOURCES
    src/Application/SwitchEvent/SwitchEvent.cpp
    src/Application/LightRequest/LightRequest.cpp
    src/Application/FLM/FLM_Application.cpp
    src/Application/Headlight/Headlight.cpp
    src/Application/SafetyMonitor/SafetyMonitor.cpp
)

set(BSW_SOURCES
    src/BSW/E2E/E2E_P01.cpp
    src/BSW/WdgM/WdgM.cpp
    src/BSW/Dem/Dem.cpp
    src/BSW/Com/Com.cpp
    src/BSW/BswM/BswM.cpp
)

set(MCAL_SOURCES
    src/MCAL/Adc/Adc.cpp
    src/MCAL/Dio/Dio.cpp
    src/MCAL/Can/Can.cpp
)

set(ALL_LIBRARY_SOURCES
    ${APPLICATION_SOURCES}
    ${BSW_SOURCES}
    ${MCAL_SOURCES}
)

##############################################################################
# Library Target (for testing)
##############################################################################

add_library(flm_lib STATIC ${ALL_LIBRARY_SOURCES})
target_include_directories(flm_lib PUBLIC ${INCLUDE_DIRS})

##############################################################################
# Main Application Target
##############################################################################

add_executable(flm_application
    src/main.cpp
)

target_include_directories(flm_application PRIVATE ${INCLUDE_DIRS})
target_link_libraries(flm_application PRIVATE flm_lib)

# Link pthread on Unix-like systems
if(UNIX)
    target_link_libraries(flm_application PRIVATE pthread)
endif()

##############################################################################
# Unit Tests
##############################################################################

if(BUILD_TESTS)
    enable_testing()

    # Try to find GTest
    find_package(GTest QUIET)

    if(GTest_FOUND)
        message(STATUS "Google Test found - building unit tests")

        set(TEST_SOURCES
            test/test_E2E.cpp
            test/test_SwitchEvent.cpp
            test/test_LightRequest.cpp
            test/test_FLM.cpp
            test/test_SafetyMonitor.cpp
        )

        add_executable(flm_tests ${TEST_SOURCES})
        target_include_directories(flm_tests PRIVATE ${INCLUDE_DIRS})
        target_link_libraries(flm_tests
            PRIVATE
            flm_lib
            GTest::gtest
            GTest::gtest_main
        )

        include(GoogleTest)
        gtest_discover_tests(flm_tests)

    else()
        message(STATUS "Google Test not found - unit tests will not be built")
        message(STATUS "Install GTest: brew install googletest (macOS) or apt-get install libgtest-dev (Linux)")
    endif()
endif()

##############################################################################
# Install Target
##############################################################################

install(TARGETS flm_application
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

##############################################################################
# Custom Targets
##############################################################################

# Run the application
add_custom_target(run
    COMMAND flm_application
    DEPENDS flm_application
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running FLM Application..."
)

# Clean build artifacts
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMENT "Cleaning all build artifacts..."
)

##############################################################################
# Print Configuration Summary
##############################################################################

message(STATUS "")
message(STATUS "=== AUTOSAR FLM Safety Use Case Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "Enable Warnings: ${ENABLE_WARNINGS}")
message(STATUS "Enable Coverage: ${ENABLE_COVERAGE}")
message(STATUS "")
