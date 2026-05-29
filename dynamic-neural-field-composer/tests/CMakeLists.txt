option(DNF_COMPOSER_BUILD_TESTS "Build unit tests" ON)
if(DNF_COMPOSER_BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)

    add_executable(dnf_composer_tests
            # application
            "application/test_application.cpp"
            # elements
            "elements/test_activation_function.cpp"
            "elements/test_element.cpp"
            "elements/test_element_factory.cpp"
            "elements/test_field_coupling.cpp"
            "elements/test_gauss_field_coupling.cpp"
            "elements/test_gauss_stimulus.cpp"
            "elements/test_kernels.cpp"
            "elements/test_neural_field.cpp"
            "elements/test_normal_noise.cpp"
            "elements/test_correlated_normal_noise.cpp"
            "elements/test_boost_stimulus.cpp"
            "elements/test_memory_trace.cpp"
            "elements/test_neural_field_2d.cpp"
            "elements/test_gauss_stimulus_2d.cpp"
            "elements/test_gauss_kernel_2d.cpp"
            "elements/test_oscillatory_kernel_2d.cpp"
            "elements/test_timed_gauss_stimulus.cpp"
            "elements/test_timed_gauss_stimulus_2d.cpp"
            "elements/test_boost_stimulus_2d.cpp"
            "elements/test_normal_noise_2d.cpp"
            "elements/test_correlated_normal_noise_2d.cpp"
            "elements/test_asymmetric_gauss_kernel_2d.cpp"
            "elements/test_memory_trace_2d.cpp"
            "elements/test_mexican_hat_kernel_2d.cpp"
            "elements/test_change_dimensions.cpp"
            # simulation
            "simulation/test_simulation.cpp"
            "simulation/test_simulation_extended.cpp"
            "simulation/test_simulation_file_manager.cpp"
            "simulation/test_simulation_recorder.cpp"
            # tools
            "tools/test_logger.cpp"
            "tools/test_math.cpp"
            "tools/test_utils.cpp"
            # element_parameters
            "element_parameters/test_element_parameters.cpp"
            # exceptions
            "exceptions/test_exception.cpp"
    )
    target_include_directories(dnf_composer_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(dnf_composer_tests PRIVATE
            ${CMAKE_PROJECT_NAME}
            GTest::gtest
            GTest::gtest_main
            imgui::imgui
            imgui-platform-kit
    )

    include(GoogleTest)
    gtest_discover_tests(dnf_composer_tests)
endif()
