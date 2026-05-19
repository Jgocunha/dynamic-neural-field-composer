function(add_example_executable target_name source_file)
    add_executable(${target_name} "${source_file}" ${DNF_ICON_RC})
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${target_name} PRIVATE
            imgui::imgui
            imgui-platform-kit
            ${CMAKE_PROJECT_NAME})
    install(TARGETS ${target_name} RUNTIME DESTINATION ${DNF_COMPOSER_RUNTIME_INSTALL_DIR})
endfunction()

add_example_executable(example_travelling_bump      travelling_bump.cpp)
add_example_executable(example_travelling_bump_2d   travelling_bump_2d.cpp)
add_example_executable(example_timed_stimuli        timed_stimuli.cpp)
add_example_executable(example_timed_stimuli_2d     timed_stimuli_2d.cpp)


add_example_executable(ex_comp_act_selection ex_complementary_action_selection.cpp)
add_example_executable(ex_field_couplings ex_field_couplings.cpp)
add_example_executable(ex_gauss_and_field_couplings ex_gauss_and_field_couplings.cpp)
add_example_executable(ex_field_coupling_learning ex_field_coupling_learning.cpp)
add_example_executable(ex_boost_stimulus ex_boost_stimulus.cpp)
add_example_executable(ex_memory_trace ex_memory_trace.cpp)
add_example_executable(ex_neural_field_2d ex_neural_field_2d.cpp)
add_example_executable(ex_oscillatory_kernel_2d ex_oscillatory_kernel_2d.cpp)

add_example_executable(ex_boost_stimulus_2d ex_boost_stimulus_2d.cpp)
add_example_executable(ex_competitive_selection ex_competitive_selection.cpp)
