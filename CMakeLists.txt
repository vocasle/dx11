cmake_minimum_required(VERSION 3.15)

project(dx11_junior_roadmap)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
add_subdirectory(bloom)
add_subdirectory(common)
