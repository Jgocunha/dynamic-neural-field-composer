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
            "elements/test_resize.cpp"
            "elements/test_resize_2d.cpp"
            "elements/test_collapse.cpp"
            "elements/test_expand.cpp"
            # simulation
            "simulation/test_simulation.cpp"
            "simulation/test_simulation_extended.cpp"
            "simulation/test_simulation_file_manager.cpp"
            "simulation/test_simulation_recorder.cpp"
            "simulation/test_thread_safety.cpp"
            # tools
            "tools/test_logger.cpp"
            "tools/test_math.cpp"
            "tools/test_utils.cpp"
            # validation (field-dynamics regression vs vendored reference data)
            "validation/test_field_dynamics_1d.cpp"
            "validation/test_field_dynamics_2d.cpp"
            # element_parameters
            "element_parameters/test_element_parameters.cpp"
            # exceptions
            "exceptions/test_exception.cpp"
    )
    target_include_directories(dnf_composer_tests PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/validation)

    # Path to the vendored field-dynamics reference data (sim JSONs + expected CSVs).
    target_compile_definitions(dnf_composer_tests PRIVATE
            VALIDATION_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/validation/data")
    target_link_libraries(dnf_composer_tests PRIVATE
            ${CMAKE_PROJECT_NAME}
            GTest::gtest
            GTest::gtest_main
            imgui::imgui
            imgui-platform-kit
    )

    include(GoogleTest)
    gtest_discover_tests(dnf_composer_tests DISCOVERY_MODE PRE_TEST)

    # Standalone throughput benchmark (manual perf run; not a unit test, so it is
    # NOT registered with gtest_discover_tests). Appends a dated session block to
    # tests/benchmark/results.md.
    add_executable(dnf_composer_benchmark "benchmark/benchmark_main.cpp")
    target_include_directories(dnf_composer_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_compile_definitions(dnf_composer_benchmark PRIVATE
            BENCHMARK_RESULTS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/benchmark/results.md")
    target_link_libraries(dnf_composer_benchmark PRIVATE
            ${CMAKE_PROJECT_NAME}
            imgui::imgui
            imgui-platform-kit
    )

    # Per-step method profiler (manual perf run; not a unit test). Appends a dated
    # session block to tests/profiler/profile.md.
    add_executable(dnf_composer_profiler "profiler/profiler_main.cpp")
    target_include_directories(dnf_composer_profiler PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_compile_definitions(dnf_composer_profiler PRIVATE
            PROFILER_RESULTS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/profiler/profile.md")
    target_link_libraries(dnf_composer_profiler PRIVATE
            ${CMAKE_PROJECT_NAME}
            imgui::imgui
            imgui-platform-kit
    )
endif()
