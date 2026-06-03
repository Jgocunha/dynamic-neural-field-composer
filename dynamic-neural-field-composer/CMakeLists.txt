cmake_minimum_required(VERSION 3.20...3.30)

project(dynamic-neural-field-composer
        DESCRIPTION "A C++20 library and interactive application for building and simulating Dynamic Neural Field (DNF) architectures."
        LANGUAGES CXX
)

# Define library version
set(DNF_COMPOSER_VERSION_MAJOR 2 CACHE STRING "major version" FORCE)
set(DNF_COMPOSER_VERSION_MINOR 8 CACHE STRING "minor version" FORCE)
set(DNF_COMPOSER_VERSION_PATCH 0 CACHE STRING "patch version" FORCE)
set(DNF_COMPOSER_VERSION ${DNF_COMPOSER_VERSION_MAJOR}.${DNF_COMPOSER_VERSION_MINOR}.${DNF_COMPOSER_VERSION_PATCH} CACHE STRING "version" FORCE)

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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
set(simulation_headers
        "include/simulation/simulation.h"
        "include/simulation/simulation_file_manager.h"
        "include/simulation/simulation_recorder.h"
)
set(visualization_headers
        "include/visualization/visualization.h"
        "include/visualization/plot.h"
        "include/visualization/plot_parameters.h"
        "include/visualization/lineplot.h"
        "include/visualization/heatmap.h"
)
set(application_headers
        "include/application/application.h"
)
set(elements_headers
        "include/elements/activation_function.h"
        "include/elements/element.h"
        "include/elements/element_factory.h"
        
        "include/elements/field_coupling.h"
        "include/elements/gauss_field_coupling.h"
        "include/elements/gauss_kernel.h"
        "include/elements/gauss_stimulus.h"
        "include/elements/kernel.h"
        "include/elements/mexican_hat_kernel.h"
        "include/elements/neural_field.h"
        "include/elements/normal_noise.h"
        "include/elements/oscillatory_kernel.h"
        "include/elements/asymmetric_gauss_kernel.h"
        "include/elements/boost_stimulus.h"
        "include/elements/memory_trace.h"
        "include/elements/correlated_normal_noise.h"

        "include/elements/neural_field_2d.h"
        "include/elements/gauss_stimulus_2d.h"
        "include/elements/gauss_kernel_2d.h"
        "include/elements/mexican_hat_kernel_2d.h"
        "include/elements/normal_noise_2d.h"
        "include/elements/oscillatory_kernel_2d.h"
        "include/elements/timed_gauss_stimulus.h"
        "include/elements/timed_gauss_stimulus_2d.h"
        "include/elements/boost_stimulus_2d.h"
        "include/elements/correlated_normal_noise_2d.h"
        "include/elements/asymmetric_gauss_kernel_2d.h"
        "include/elements/memory_trace_2d.h"
)
set(element_parameters_headers
        "include/element_parameters/element_parameters.h"
)

set(tools_headers
        "include/tools/logger.h"
        "include/tools/math.h"
        "include/tools/profiling.h"
        "include/tools/utils.h"
        "include/tools/file_dialog.h"
)
set(exceptions_headers
        "include/exceptions/exception.h"
)
set(user_interface_headers
        "include/user_interface/field_metrics_window.h"
        "include/user_interface/element_window.h"
        "include/user_interface/plot_control_window.h"
        "include/user_interface/simulation_window.h"
        "include/user_interface/node_graph_window.h"
        "include/user_interface/main_menu_bar.h"
        "include/user_interface/widgets.h"
        "include/user_interface/plots_window.h"
        "include/user_interface/log_window.h"
        "include/user_interface/help_window.h"
        "include/user_interface/static_layout.h"
        "include/user_interface/status_bar_window.h"
        "include/user_interface/control_bar_window.h"

        "include/user_interface/node_utilities/builders.h"
        "include/user_interface/node_utilities/drawing.h"
        "include/user_interface/node_utilities/node_widgets.h"

        "include/user_interface/fonts/IconsFontAwesome6.h"
        "include/user_interface/fonts/fa.h"
)

set(header 
    ${simulation_headers}
    ${visualization_headers}
    ${application_headers}
    ${elements_headers}
    ${element_parameters_headers}
    ${tools_headers}
    ${exceptions_headers}
    ${user_interface_headers}
    ${user_interface_headers}
)

# Set source files
set(src 
        "src/application/application.cpp"
        "src/application/style.cpp"

        "src/simulation/simulation.cpp"
        "src/simulation/simulation_file_manager.cpp"
        "src/simulation/simulation_recorder.cpp"

        "src/visualization/visualization.cpp"
        "src/visualization/plot.cpp"
        "src/visualization/plot_parameters.cpp"
        "src/visualization/lineplot.cpp"
        "src/visualization/heatmap.cpp"

        "src/elements/activation_function.cpp"
        "src/elements/element.cpp"
        "src/elements/element_factory.cpp"
        "src/elements/field_coupling.cpp"
        "src/elements/gauss_field_coupling.cpp"
        "src/elements/gauss_kernel.cpp"
        "src/elements/gauss_stimulus.cpp"
        "src/elements/kernel.cpp"
        "src/elements/mexican_hat_kernel.cpp"
        "src/elements/oscillatory_kernel.cpp"
        "src/elements/neural_field.cpp"
        "src/elements/normal_noise.cpp"
        "src/elements/asymmetric_gauss_kernel.cpp"
        "src/elements/boost_stimulus.cpp"
        "src/elements/memory_trace.cpp"
        "src/elements/neural_field_2d.cpp"
        "src/elements/gauss_stimulus_2d.cpp"
        "src/elements/gauss_kernel_2d.cpp"
        "src/elements/mexican_hat_kernel_2d.cpp"
        "src/elements/normal_noise_2d.cpp"
        "src/elements/oscillatory_kernel_2d.cpp"
        "src/elements/timed_gauss_stimulus.cpp"
        "src/elements/timed_gauss_stimulus_2d.cpp"
        "src/elements/boost_stimulus_2d.cpp"
        "src/elements/correlated_normal_noise.cpp"
        "src/elements/correlated_normal_noise_2d.cpp"
        "src/elements/asymmetric_gauss_kernel_2d.cpp"
        "src/elements/memory_trace_2d.cpp"
        "src/elements/resize.cpp"
        "src/elements/resize_2d.cpp"

        "src/element_parameters/element_parameters.cpp"

        "src/tools/math.cpp"
        "src/tools/profiling.cpp"
        "src/tools/utils.cpp"
        "src/tools/logger.cpp"

        "src/exceptions/exception.cpp"

        "src/user_interface/plot_control_window.cpp"
        "src/user_interface/simulation_window.cpp"
        "src/user_interface/field_metrics_window.cpp"
        "src/user_interface/element_window.cpp"
        "src/user_interface/node_graph_window.cpp"
        "src/user_interface/main_menu_bar.cpp"
        "src/user_interface/widgets.cpp"
        "src/user_interface/plots_window.cpp"
        "src/user_interface/log_window.cpp"
        "src/user_interface/help_window.cpp"
        "src/user_interface/static_layout.cpp"
        "src/user_interface/status_bar_window.cpp"
        "src/user_interface/control_bar_window.cpp"

        "src/user_interface/node_utilities/builders.cpp"
        "src/user_interface/node_utilities/drawing.cpp"
        "src/user_interface/node_utilities/node_widgets.cpp"
)

# Define library target
add_library(${CMAKE_PROJECT_NAME} ${header} ${src})
target_include_directories(${CMAKE_PROJECT_NAME} 
    PUBLIC $<INSTALL_INTERFACE:include> 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include 
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

# OpenGL must be found before importing imgui-platform-kit so that the
# OpenGL::GL target exists when CMake resolves its exported link interface.
find_package(OpenGL REQUIRED)

# Setup imgui-platform-kit
find_package(imgui-platform-kit REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE imgui-platform-kit)

if(WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE psapi)
endif()

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
    DNF_COMPOSER=1
    DNF_COMPOSER_VERSION_MAJOR=${DNF_COMPOSER_VERSION_MAJOR}
    DNF_COMPOSER_VERSION_MINOR=${DNF_COMPOSER_VERSION_MINOR}
    DNF_COMPOSER_VERSION_PATCH=${DNF_COMPOSER_VERSION_PATCH}
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${CMAKE_PROJECT_NAME}-${DNF_COMPOSER_VERSION}"
    POSITION_INDEPENDENT_CODE ON
)

# Install the library (binaries)
install(TARGETS ${CMAKE_PROJECT_NAME} EXPORT ${CMAKE_PROJECT_NAME}Targets
    RUNTIME DESTINATION ${DNF_COMPOSER_RUNTIME_INSTALL_DIR}
    LIBRARY DESTINATION ${DNF_COMPOSER_LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${DNF_COMPOSER_ARCHIVE_INSTALL_DIR}
    INCLUDES DESTINATION include/dnf_composer # Explicitly set the install include path
)

# Package version configuration
include(CMakePackageConfigHelpers)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}-config-version.cmake")
write_basic_package_version_file(${version_file}
    VERSION ${DNF_COMPOSER_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Install the headers 
install(FILES ${simulation_headers} DESTINATION ${DNF_COMPOSER_INC_INSTALL_DIR}/dnf_composer/simulation)
install(FILES ${visualization_headers} DESTINATION ${DNF_COMPOSER_INC_INSTALL_DIR}/dnf_composer/visualization)
install(FILES ${application_headers} DESTINATION ${DNF_COMPOSER_INC_INSTALL_DIR}/dnf_composer/application)
install(FILES ${elements_headers} DESTINATION ${DNF_COMPOSER_INC_INSTALL_DIR}/dnf_composer/elements)
install(FILES ${tools_headers} DESTINATION ${DNF_COMPOSER_INC_INSTALL_DIR}/dnf_composer/tools)
install(FILES ${exceptions_headers} DESTINATION ${DNF_COMPOSER_INC_INSTALL_DIR}/dnf_composer/exceptions)
install(FILES ${user_interface_headers} DESTINATION ${DNF_COMPOSER_INC_INSTALL_DIR}/dnf_composer/user_interface)
install(FILES ${element_parameters_headers} DESTINATION ${DNF_COMPOSER_INC_INSTALL_DIR}/dnf_composer/element_parameters)

# Install runtime resources (fonts, icons, images, layouts)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/" DESTINATION resources)

# Install sample data (simulation JSONs, coupling weights)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/" DESTINATION data)

install(EXPORT ${CMAKE_PROJECT_NAME}Targets DESTINATION ${DNF_COMPOSER_CMAKE_CONFIG_INSTALL_DIR}
    FILE ${CMAKE_PROJECT_NAME}-config.cmake
)

# Embed the application icon into each executable on Windows via the .rc file.
# On Linux the window icon is set at runtime (see application.cpp IconParameters);
# there is no equivalent binary embedding for ELF executables.
if(WIN32)
    set(DNF_ICON_RC "${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.rc")
else()
    set(DNF_ICON_RC "")
endif()

# Main launchers
set(LAUNCHER dnf-composer-static)
add_executable(${LAUNCHER} WIN32 "src/dynamic-neural-field-composer-static.cpp" ${DNF_ICON_RC})
target_include_directories(${LAUNCHER} PRIVATE include)
target_link_libraries(${LAUNCHER} PRIVATE
imgui::imgui
imgui-platform-kit
${CMAKE_PROJECT_NAME})
if(WIN32)
    target_link_options(${LAUNCHER} PRIVATE /ENTRY:mainCRTStartup)
endif()
install(TARGETS ${LAUNCHER} RUNTIME DESTINATION ${DNF_COMPOSER_RUNTIME_INSTALL_DIR})

set(LAUNCHER dnf-composer-dynamic)
add_executable(${LAUNCHER} WIN32 "src/dynamic-neural-field-composer-dynamic.cpp" ${DNF_ICON_RC})
target_include_directories(${LAUNCHER} PRIVATE include)
target_link_libraries(${LAUNCHER} PRIVATE
        imgui::imgui
        imgui-platform-kit
        ${CMAKE_PROJECT_NAME})
if(WIN32)
    target_link_options(${LAUNCHER} PRIVATE /ENTRY:mainCRTStartup)
endif()
install(TARGETS ${LAUNCHER} RUNTIME DESTINATION ${DNF_COMPOSER_RUNTIME_INSTALL_DIR})

# Examples
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

# Documentation (Doxygen)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
)
find_package(Doxygen OPTIONAL_COMPONENTS dot)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Tests
enable_testing()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()