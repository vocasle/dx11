cmake_minimum_required(VERSION 3.15)

project(dx11_junior_roadmap)

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

add_subdirectory(dx11)
add_subdirectory(shadows)
