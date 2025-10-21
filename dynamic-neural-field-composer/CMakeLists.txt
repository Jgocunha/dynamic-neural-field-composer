cmake_minimum_required(VERSION 3.20...3.30)

project(dynamic-neural-field-composer
        DESCRIPTION "A C++ application to simulate Dynamic Neural Field Architectures"
        LANGUAGES CXX
)

# Define library version
set(DNF_COMPOSER_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
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
set(simulation_headers
        "include/simulation/simulation.h"
        "include/simulation/simulation_file_manager.h"
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
)
set(element_parameters_headers
        "include/element_parameters/element_parameters.h"
#"include/element_parameters/field_coupling_parameters.h"
##"include/element_parameters/gauss_field_coupling_parameters.h"
#"include/element_parameters/gauss_kernel_parameters.h"
#"include/element_parameters/gauss_stimulus_parameters.h"
#"include/element_parameters/mexican_hat_kernel_parameters.h"
#"include/element_parameters/neural_field_parameters.h"
#"include/element_parameters/normal_noise_parameters.h"
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
        "include/user_interface/main_window.h"
        "include/user_interface/widgets.h"
        "include/user_interface/plots_window.h"
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

        "src/simulation/simulation.cpp"
        "src/simulation/simulation_file_manager.cpp"

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

        "src/element_parameters/element_parameters.cpp"
#"src/element_parameters/field_coupling_parameters.cpp"
##"include/element_parameters/gauss_field_coupling_parameters.h"
#"src/element_parameters/gauss_kernel_parameters.cpp"
#"src/element_parameters/gauss_stimulus_parameters.cpp"
#"src/element_parameters/lateral_interactions_parameters.cpp"
#"src/element_parameters/mexican_hat_kernel_parameters.cpp"
#"src/element_parameters/neural_field_parameters.cpp"
#"src/element_parameters/normal_noise_parameters.cpp"

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
        "src/user_interface/main_window.cpp"
        "src/user_interface/widgets.cpp"
        "src/user_interface/plots_window.cpp"
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

install(EXPORT ${CMAKE_PROJECT_NAME}Targets DESTINATION ${DNF_COMPOSER_CMAKE_CONFIG_INSTALL_DIR}
    FILE ${CMAKE_PROJECT_NAME}-config.cmake
)


# Main launcher
set(LAUNCHER launcher)
add_executable(${LAUNCHER} "src/dynamic-neural-field-composer.cpp")
target_include_directories(${LAUNCHER} PRIVATE include)
target_link_libraries(${LAUNCHER} PRIVATE
imgui::imgui
imgui-platform-kit 
${CMAKE_PROJECT_NAME})

# Function to add example executables
function(add_example_executable target_name source_file)
    add_executable(${target_name} "examples/${source_file}")
    target_include_directories(${target_name} PRIVATE include)
    target_link_libraries(${target_name} PRIVATE
            imgui::imgui
            imgui-platform-kit
            ${CMAKE_PROJECT_NAME})
endfunction()

# Define all examples
add_example_executable(ex_comp_act_selection ex_complementary_action_selection.cpp)
add_example_executable(ex_asymmetric_gauss_kernel ex_asymmetric_gauss_kernel.cpp)
add_example_executable(ex_two_robot_team ex_two_robot_team.cpp)
add_example_executable(ex_field_couplings ex_field_couplings.cpp)
add_example_executable(ex_gauss_and_field_couplings ex_gauss_and_field_couplings.cpp)
add_example_executable(ex_field_coupling_learning ex_field_coupling_learning.cpp)