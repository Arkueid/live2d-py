file(GLOB shader_files src/*)

# Shader compilation
foreach(shader ${shader_files})
    get_filename_component(file_name ${shader} NAME)
    if(file_name MATCHES "common.glsl")
        continue()
    endif()
    get_filename_component(full_path ${shader} ABSOLUTE)
    set(output_dir ${CMAKE_CURRENT_BINARY_DIR}/compiledShaders)
    string(REGEX REPLACE \\.frag|\\.vert "" output_file ${file_name})
    set(output_file ${output_dir}/${output_file}.spv)
    set(compiled_shaders_framework ${compiled_shaders_framework} ${output_file})
    set(compiled_shaders_framework ${compiled_shaders_framework} PARENT_SCOPE)
    set_source_files_properties(${shader} PROPERTIES HEADER_FILE_ONLY TRUE)

    if(WIN32)
        add_custom_command(
            OUTPUT ${output_file}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${output_dir}
            COMMAND $ENV{VK_SDK_PATH}/Bin/glslc.exe ${full_path} -o ${output_file}
            DEPENDS ${full_path}
        )
    endif()
    if(UNIX AND NOT APPLE)
        add_custom_command(
            OUTPUT ${output_file}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${output_dir}
            COMMAND glslc ${full_path} -o ${output_file}
            DEPENDS ${full_path}
        )
    endif()
endforeach()

source_group("shaders" FILES ${shader_files})
add_custom_target(
    FrameworkShaders ALL
    DEPENDS ${compiled_shaders_framework}
    SOURCES ${shader_files}
)
