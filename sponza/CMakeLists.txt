set(app_name "sponza")

set(app_source_files "src/main.cpp")
set(app_header_files "include/main.h")

add_executable("${app_name}" "${app_source_files}" "${app_header_files}")

target_include_directories("${app_name}" PRIVATE "include" "${CMAKE_SOURCE_DIR}/assimp/include")

target_link_libraries("${app_name}"
    "${CMAKE_SOURCE_DIR}/assimp/lib/assimp-vc143-mt.lib"
    d3d11.lib
    dxgi.lib
    dxguid.lib)

target_compile_definitions("${app_name}" 
    PUBLIC NOMINMAX=1
	PUBLIC WITH_IMGUI=1
    PRIVATE "${app_name}" ASSETS_ROOT="${CMAKE_BINARY_DIR}/assets")

if (MSVC)
	target_compile_definitions("${app_name}" PRIVATE SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
	target_compile_definitions("${app_name}" PRIVATE SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}")
endif()