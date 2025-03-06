cmake_minimum_required(VERSION 3.15)

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
    #"include/elements/field_coupling.h"
    #"include/elements/gauss_field_coupling.h"
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
   # "src/elements/field_coupling.cpp"
    #"src/elements/gauss_field_coupling.cpp" 
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


# Entry point
set(ENTRY_POINT entrypoint)
add_executable(${ENTRY_POINT} "examples/entry.cpp")
target_include_directories(${ENTRY_POINT} PRIVATE include)
target_link_libraries(${ENTRY_POINT} PRIVATE 
imgui::imgui
imgui-platform-kit 
${CMAKE_PROJECT_NAME})

# Testbench
set(TESTBENCH testbench)
add_executable(${TESTBENCH} "examples/testbench.cpp")
target_include_directories(${TESTBENCH} PRIVATE include)
target_link_libraries(${TESTBENCH} PRIVATE 
imgui::imgui
imgui-platform-kit 
${CMAKE_PROJECT_NAME})

# Complementary action selection architecture
set(COMP_ACTION_SELECTION_ARCH comp_act_selection)
add_executable(${COMP_ACTION_SELECTION_ARCH} "examples/complementary_action_selection.cpp")
target_include_directories(${COMP_ACTION_SELECTION_ARCH} PRIVATE include)
target_link_libraries(${COMP_ACTION_SELECTION_ARCH} PRIVATE 
imgui::imgui
imgui-platform-kit 
${CMAKE_PROJECT_NAME})

# Test asymmetric gauss kernel
set(TEST_AGK test_agk)
add_executable(${TEST_AGK} "examples/test_agk.cpp")
target_include_directories(${TEST_AGK} PRIVATE include)
target_link_libraries(${TEST_AGK} PRIVATE 
imgui::imgui
imgui-platform-kit 
${CMAKE_PROJECT_NAME})


## Example test_plot_control_window
#set(TEST_PLOT_CONTROL_WINDOW_EXAMPLE t-plot_control_window-example)
#add_executable(${TEST_PLOT_CONTROL_WINDOW_EXAMPLE} "examples/test_plot_control_window.cpp")
#target_include_directories(${TEST_PLOT_CONTROL_WINDOW_EXAMPLE} PRIVATE include)
#target_link_libraries(${TEST_PLOT_CONTROL_WINDOW_EXAMPLE} PRIVATE 
#imgui::imgui
#imgui-platform-kit 
#${CMAKE_PROJECT_NAME})
#
## Example test_couplings
#set(TEST_COUPLINGS_EXAMPLE t-couplings-example)
#add_executable(${TEST_COUPLINGS_EXAMPLE} "examples/test_couplings.cpp")
#target_include_directories(${TEST_COUPLINGS_EXAMPLE} PRIVATE include)
#target_link_libraries(${TEST_COUPLINGS_EXAMPLE} PRIVATE 
#imgui::imgui
#imgui-platform-kit 
#${CMAKE_PROJECT_NAME})
#
## Example test_factory
#set(TEST_FACTORY_EXAMPLE t-factory-example)
#add_executable(${TEST_FACTORY_EXAMPLE} "examples/test_factory.cpp")
#target_include_directories(${TEST_FACTORY_EXAMPLE} PRIVATE include)
#target_link_libraries(${TEST_FACTORY_EXAMPLE} PRIVATE 
#imgui::imgui
#imgui-platform-kit 
#${CMAKE_PROJECT_NAME})
#
## Example test_nodegraph
#set(TEST_NODEGRAPH_EXAMPLE t-nodegraph-example)
#add_executable(${TEST_NODEGRAPH_EXAMPLE} "examples/test_nodegraph.cpp")
#target_include_directories(${TEST_NODEGRAPH_EXAMPLE} PRIVATE include)
#target_link_libraries(${TEST_NODEGRAPH_EXAMPLE} PRIVATE 
#imgui::imgui
#imgui-platform-kit 
#${CMAKE_PROJECT_NAME})
#
## Example test_simulation
#set(TEST_SIMULATION_EXAMPLE t-simulation-example)
#add_executable(${TEST_SIMULATION_EXAMPLE} "examples/test_simulation.cpp")
#target_include_directories(${TEST_SIMULATION_EXAMPLE} PRIVATE include)
#target_link_libraries(${TEST_SIMULATION_EXAMPLE} PRIVATE 
#imgui::imgui
#imgui-platform-kit 
#${CMAKE_PROJECT_NAME})
