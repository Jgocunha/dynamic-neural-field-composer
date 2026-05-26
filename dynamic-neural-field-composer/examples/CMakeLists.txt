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


add_example_executable(ex_field_couplings ex_field_couplings.cpp)
add_example_executable(ex_gauss_and_field_couplings ex_gauss_and_field_couplings.cpp)
add_example_executable(ex_field_coupling_learning ex_field_coupling_learning.cpp)




