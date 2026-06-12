function(add_example_executable target_name source_file)
    add_executable(${target_name} "${source_file}" ${DNF_ICON_RC})
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${target_name} PRIVATE
            imgui::imgui
            imgui-platform-kit
            ${CMAKE_PROJECT_NAME})
    install(TARGETS ${target_name} RUNTIME DESTINATION ${DNF_COMPOSER_RUNTIME_INSTALL_DIR})
endfunction()


add_example_executable(example_detection_instability    detection_instability.cpp)
add_example_executable(example_detection_instability_2d detection_instability_2d.cpp)
add_example_executable(example_memory_instability       memory_instability.cpp)
add_example_executable(example_memory_instability_2d    memory_instability_2d.cpp)
add_example_executable(example_selection_instability    selection_instability.cpp)
add_example_executable(example_selection_instability_2d selection_instability_2d.cpp)
add_example_executable(example_boost_detection          boost_detection.cpp)
add_example_executable(example_boost_detection_2d       boost_detection_2d.cpp)
add_example_executable(example_memory_trace             memory_trace.cpp)
add_example_executable(example_memory_trace_2d          memory_trace_2d.cpp)
add_example_executable(example_travelling_bump          travelling_bump.cpp)
add_example_executable(example_travelling_bump_2d       travelling_bump_2d.cpp)
add_example_executable(example_multi_peak               multi_peak.cpp)
add_example_executable(example_multi_peak_2d            multi_peak_2d.cpp)
add_example_executable(example_weighted_field_coupling  weighted_field_coupling.cpp)
add_example_executable(example_gaussian_field_coupling  gaussian_field_coupling.cpp)
add_example_executable(example_hebbian_learning         hebbian_learning.cpp)
add_example_executable(example_resize                    resize.cpp)
add_example_executable(example_dimensionality_collapse_expand dimensionality_collapse_expand.cpp)
#add_example_executable(example_oja_learning             oja_learning.cpp)
add_example_executable(example_delta_rule_learning      delta_rule_learning.cpp)



