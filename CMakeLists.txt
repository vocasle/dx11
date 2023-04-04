cmake_minimum_required(VERSION 3.15)

project(dx11_junior_roadmap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(FXC_EXE fxc REQUIRED)

file(GLOB_RECURSE models "${PROJECT_SOURCE_DIR}/assets/meshes/*.obj")
file(GLOB_RECURSE textures "${PROJECT_SOURCE_DIR}/assets/textures/*.png" "${PROJECT_SOURCE_DIR}/assets/textures/*.jpg")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/assets/meshes")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/assets/textures")

foreach(file ${models})
	get_filename_component(filename ${file} NAME)
	set(out_dir "${CMAKE_BINARY_DIR}/assets/meshes/")
	message(STATUS "Copying ${file} to ${out_dir}${filename}")
	file(COPY_FILE ${file} ${out_dir}${filename} ONLY_IF_DIFFERENT)
endforeach()

foreach(file ${textures})
	get_filename_component(filename ${file} NAME)
	set(out_dir "${CMAKE_BINARY_DIR}/assets/textures/")
	message(STATUS "Copying ${file} to ${out_dir}${filename}")
	file(COPY_FILE ${file} ${out_dir}${filename} ONLY_IF_DIFFERENT)
endforeach()

set(assets_root "${CMAKE_BINARY_DIR}/assets")

find_program(FXC_EXE NAMES "fxc.exe" REQUIRED)

function(get_shader_profile FILE_NAME SHADER_PROFILE)
    set(POS -1)
    string(FIND ${FILE_NAME} "VS" POS)
    if (POS EQUAL -1)
        set(SHADER_PROFILE "ps_5_0" PARENT_SCOPE)
    else()
        set(SHADER_PROFILE "vs_5_0" PARENT_SCOPE)
    endif()
endfunction()

# Taken from https://github.com/DanOlivier/Vulkan/blob/cmake-support/CMakeLists.txt
function(compile_shaders EXAMPLE_NAME)
    if(NOT ARGN)
        return()
    endif()
    message("SHADERS:")
    foreach(f ${ARGN})
        message("\t- ${f}")
    endforeach(f)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shaders")
    foreach(SHADER ${ARGN})
        get_filename_component(FILE_NAME ${SHADER} NAME_WE)
        set(SHADER_PROFILE "")
        get_shader_profile(${FILE_NAME} SHADER_PROFILE)

        set(output_file ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME}.cso)
        set(compiled_shaders ${compiled_shaders} ${output_file})
        set(compiled_shaders ${compiled_shaders} PARENT_SCOPE)
        add_custom_command(
            OUTPUT ${output_file}
            COMMAND ${FXC_EXE} /T ${SHADER_PROFILE} /E main /WX /Zi /Od /Fo ${output_file} ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
            COMMENT "Compiling shader ${output_file} with profile ${SHADER_PROFILE}"
        )
    endforeach()
    #message(STATUS "add_custom_target: shaders-${EXAMPLE_NAME}")
    add_custom_target(shaders-${EXAMPLE_NAME} ALL DEPENDS ${compiled_shaders})
    add_dependencies(${EXAMPLE_NAME} shaders-${EXAMPLE_NAME})
    #add_dependencies(${EXAMPLE_NAME} ${compiled_shaders})
endfunction()

add_subdirectory(imgui-1.87)
add_subdirectory(dx11)
add_subdirectory(shadows)
add_subdirectory(reflections)
add_subdirectory(particle-system)
add_subdirectory(math)
add_subdirectory(objloader)
add_subdirectory(soft-shadows)
add_subdirectory(basics)
add_subdirectory(utils)
add_subdirectory(input)
add_subdirectory(fog)
add_subdirectory(common)
add_subdirectory(test)
add_subdirectory(bloom)
add_subdirectory(sponza)
