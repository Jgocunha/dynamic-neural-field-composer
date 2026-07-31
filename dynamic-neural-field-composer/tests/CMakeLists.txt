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
            "elements/test_spectral_dispatch_2d.cpp"
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
            "tools/test_profiling.cpp"
            "tools/test_fft_convolution.cpp"
            # validation (field-dynamics regression vs vendored reference data)
            "validation/test_field_dynamics_1d.cpp"
            "validation/test_field_dynamics_2d.cpp"
            "validation/test_spectral_golden_2d.cpp"
            # element_parameters
            "element_parameters/test_element_parameters.cpp"
            # exceptions
            "exceptions/test_exception.cpp"
            # user_interface/ and visualization/ are auto-discovered below
            # (see the headless UI tests block for the rationale)
    )
    target_include_directories(dnf_composer_tests PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/common
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

    # --- Golden algebraic-equivalence tests -------------------------------
    # Auto-discover every tests/golden/*.cpp so parallel contributors can drop
    # new golden test files in without editing this list (CONFIGURE_DEPENDS makes
    # the build re-glob when files are added/removed). Reference headers live in
    # tests/golden/reference/ and are #included directly.
    file(GLOB golden_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/golden/*.cpp")
    target_sources(dnf_composer_tests PRIVATE ${golden_sources})
    # Absolute path to the committed golden-data directory, injected as a macro
    # so the harness can locate/refresh the frozen CSV fixtures.
    target_compile_definitions(dnf_composer_tests PRIVATE
            GOLDEN_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden/data")

    # --- Headless UI tests ------------------------------------------------
    # Same auto-discovery rationale as the golden suite: contributors drop
    # tests/user_interface/*.cpp and tests/visualization/*.cpp in without
    # touching this file. The harness in tests/user_interface/ui_test_harness.h
    # drives real render() calls against a headless ImGui context (no window,
    # no GL), so user_interface/ stops being a permanent 0%-covered block in
    # the coverage denominator.
    file(GLOB ui_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/user_interface/*.cpp")
    file(GLOB visualization_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/visualization/*.cpp")
    target_sources(dnf_composer_tests PRIVATE ${ui_sources} ${visualization_sources})
    target_include_directories(dnf_composer_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/user_interface")

    include(GoogleTest)
    gtest_discover_tests(dnf_composer_tests DISCOVERY_MODE PRE_TEST)

    # Standalone throughput benchmark (manual perf run; not a unit test, so it is
    # NOT registered with gtest_discover_tests). Appends a dated session block to
    # tests/benchmark/results.md.
    add_executable(dnf_composer_benchmark "benchmark/benchmark_main.cpp" "common/bench_env.h")
    target_include_directories(dnf_composer_benchmark PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/common)
    target_compile_definitions(dnf_composer_benchmark PRIVATE
            BENCHMARK_RESULTS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/benchmark/results.md"
            BENCH_ENV_BUILD_TYPE="$<CONFIG>"
            BENCH_ENV_FFTW_VERSION="${FFTW3_VERSION}"
            BENCH_ENV_GIT_SHA="${DNF_COMPOSER_GIT_SHA}")
    target_link_libraries(dnf_composer_benchmark PRIVATE
            ${CMAKE_PROJECT_NAME}
            imgui::imgui
            imgui-platform-kit
    )

    # Per-step method profiler (manual perf run; not a unit test). Appends a dated
    # session block to tests/profiler/profile.md.
    add_executable(dnf_composer_profiler "profiler/profiler_main.cpp" "common/bench_env.h")
    target_include_directories(dnf_composer_profiler PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/common)
    # Match the library's SIMD flags: the math.h templates instantiated in this
    # TU are COMDATs — without /arch:AVX2 the linker may keep this TU's scalar
    # copies for the whole exe, silently overriding the library's AVX2
    # instantiations (and the profiler must measure the code the library runs).
    # x86-only, same as the library's own simd_dispatch_avx2.cpp flags (see
    # CMakeLists.txt) — these flags are meaningless/invalid on ARM.
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i.86)$")
    target_compile_options(dnf_composer_profiler PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
            $<$<CXX_COMPILER_ID:GNU,Clang>:-mavx2>)
    endif()
    target_compile_definitions(dnf_composer_profiler PRIVATE
            PROFILER_RESULTS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/profiler/profile.md"
            BENCH_ENV_BUILD_TYPE="$<CONFIG>"
            BENCH_ENV_FFTW_VERSION="${FFTW3_VERSION}"
            BENCH_ENV_GIT_SHA="${DNF_COMPOSER_GIT_SHA}")
    target_link_libraries(dnf_composer_profiler PRIVATE
            ${CMAKE_PROJECT_NAME}
            imgui::imgui
            imgui-platform-kit
    )

    # Regenerates tests/validation/data/2d_spectral/expected/*.csv from the sim
    # JSONs under tests/validation/data/2d_spectral/simulations/. Not a unit
    # test (NOT registered with gtest_discover_tests) and not built as part of
    # ALL -- golden data changes only deliberately. Run manually:
    #   cmake --build <builddir> --target dnf_composer_regen_spectral_golden
    #   <builddir>/tests/.../dnf_composer_regen_spectral_golden[.exe]
    add_executable(dnf_composer_regen_spectral_golden EXCLUDE_FROM_ALL
            "validation/tools/regenerate_spectral_golden.cpp")
    target_include_directories(dnf_composer_regen_spectral_golden PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/validation)
    target_compile_definitions(dnf_composer_regen_spectral_golden PRIVATE
            VALIDATION_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/validation/data")
    target_link_libraries(dnf_composer_regen_spectral_golden PRIVATE
            ${CMAKE_PROJECT_NAME}
            imgui::imgui
            imgui-platform-kit
    )
endif()
