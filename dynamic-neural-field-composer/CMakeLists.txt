cmake_minimum_required(VERSION 3.15)

project(dynamic-neural-field-composer
        DESCRIPTION "A C++ application to simulate Dynamic Neural Field Architectures"
        LANGUAGES CXX
)

# Define library version
set(DNF_COMPOSER_VERSION_MAJOR 2 CACHE STRING "major version" FORCE)
set(DNF_COMPOSER_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set(DNF_COMPOSER_VERSION ${DNF_COMPOSER_VERSION_MAJOR}.${DNF_COMPOSER_VERSION_MINOR} CACHE STRING "version" FORCE)

set(DNF_COMPOSER_LIB_PREFIX "release")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DNF_COMPOSER_LIB_PREFIX "debug")
endif()

set(DNF_COMPOSER_INC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
set(DNF_COMPOSER_RUNTIME_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(DNF_COMPOSER_LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/${DNF_COMPOSER_LIB_PREFIX}")
set(DNF_COMPOSER_ARCHIVE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/${DNF_COMPOSER_LIB_PREFIX}")
set(DNF_COMPOSER_FRAMEWORK_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(DNF_COMPOSER_CMAKE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}")
set(DNF_COMPOSER_ADDITIONAL_FILES_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check whether VCPKG is set up in your system
if(NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR "ERROR: This project requires VCPKG.\n")
endif()

# Set VCPKG root directory
set(VCPKG_ROOT $ENV{VCPKG_ROOT})

# Include VCPKG toolchain
include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

# Set the project directory
set(PROJECT_DIR "${CMAKE_SOURCE_DIR}")
# Pass the PROJECT_DIR as a preprocessor definition
add_compile_definitions(PROJECT_DIR="${PROJECT_DIR}")

# Set the desired data output directory
set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
# Pass the OUTPUT_DIRECTORY as a preprocessor definition
add_compile_definitions(OUTPUT_DIRECTORY="${OUTPUT_DIRECTORY}")

# Set header files grouped by directories
set(headers
    "include/tools/logger.h"
    "include/simulation/simulation.h"
)

# Set source files
set(src 
    "src/tools/logger.cpp"
    "src/simulation/simulation.cpp"
)

# Define library target
add_library(${CMAKE_PROJECT_NAME} ${header} ${src})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    #$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> 
    $<INSTALL_INTERFACE:${DNF_COMPOSER_INC_INSTALL_DIR}> 
    #PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include 
)

# Setup imgui
find_package(imgui CONFIG REQUIRED)

# Setup implot
find_package(implot CONFIG REQUIRED)

# Setup imgui-node-editor
find_package(unofficial-imgui-node-editor CONFIG REQUIRED)

# Setup nlohmann-json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Setup imgui-platform-kit
find_package(imgui-platform-kit REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE imgui-platform-kit)


target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
    DNF_COMPOSER=1
    DNF_COMPOSER_VERSION_MAJOR=${DNF_COMPOSER_VERSION_MAJOR}
    DNF_COMPOSER_VERSION_MINOR=${DNF_COMPOSER_VERSION_MINOR}
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${CMAKE_PROJECT_NAME}-${DNF_COMPOSER_VERSION}"
    POSITION_INDEPENDENT_CODE ON
)

# Install the library (binaries)
install(TARGETS ${CMAKE_PROJECT_NAME} EXPORT ${CMAKE_PROJECT_NAME}Targets
    RUNTIME       DESTINATION ${DNF_COMPOSER_RUNTIME_INSTALL_DIR}
    LIBRARY       DESTINATION ${DNF_COMPOSER_LIBRARY_INSTALL_DIR}
    ARCHIVE       DESTINATION ${DNF_COMPOSER_ARCHIVE_INSTALL_DIR}
    FRAMEWORK     DESTINATION ${DNF_COMPOSER_FRAMEWORK_INSTALL_DIR}
)

# Package version configuration
include(CMakePackageConfigHelpers)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}-config-version.cmake")
write_basic_package_version_file(${version_file}
    VERSION ${DNF_COMPOSER_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Install the headers


install(EXPORT ${CMAKE_PROJECT_NAME}Targets DESTINATION ${DNF_COMPOSER_CMAKE_CONFIG_INSTALL_DIR}
    FILE ${CMAKE_PROJECT_NAME}-config.cmake
)

# Add executable project
set(EXE_PROJECT ${CMAKE_PROJECT_NAME}-exe)
add_executable(${EXE_PROJECT} "src/dynamic-neural-field-composer.cpp")
target_include_directories(${EXE_PROJECT} PRIVATE include)
target_link_libraries(${EXE_PROJECT} PRIVATE 
imgui::imgui
imgui-platform-kit 
${CMAKE_PROJECT_NAME})


# Setup Catch2
enable_testing()
find_package(Catch2 CONFIG REQUIRED)
include(CTest)
include(Catch)

# Add test project
set(TEST_PROJECT ${CMAKE_PROJECT_NAME}-test)
add_executable(${TEST_PROJECT} 
    tests/test.cpp 
)
target_include_directories(${TEST_PROJECT} PRIVATE include)
target_link_libraries(${TEST_PROJECT} PRIVATE Catch2::Catch2 Catch2::Catch2WithMain imgui-platform-kit ${CMAKE_PROJECT_NAME})
# Automatically discover and add tests
catch_discover_tests(${TEST_PROJECT})