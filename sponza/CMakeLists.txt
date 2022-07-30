set(app_name "sponza")

set(app_source_files
    "src/main.cpp"
    "src/ModelLoader.cpp"
    "src/SponzaGame.cpp")
set(app_header_files 
    "include/ModelLoader.h"
    "include/SponzaGame.h")

set(CMAKE_BUILD_TYPE Debug)

add_executable("${app_name}" WIN32 "${app_source_files}" "${app_header_files}")

set(ASSIMP_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/assimp/lib")

target_include_directories("${app_name}" PRIVATE "include" "${CMAKE_SOURCE_DIR}/assimp/include")
target_link_directories("${app_name}" PRIVATE "${ASSIMP_LIBRARY_DIR}")

if (MSVC)
    message(STATUS "Will copy to ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/.")
    add_custom_command(TARGET "${app_name}"
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${ASSIMP_LIBRARY_DIR}/assimp-vc143-mt.dll" "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/.")
else()
    add_custom_command(TARGET "${app_name}"
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${ASSIMP_LIBRARY_DIR}/assimp-vc143-mt.dll" "${CMAKE_CURRENT_BINARY_DIR}/.")
endif()

target_link_libraries("${app_name}"
    "assimp-vc143-mt.lib"
    d3d11.lib
    dxgi.lib
    dxguid.lib
    ne_math
    ne_common
    ne_utils)

target_compile_definitions("${app_name}" 
    PRIVATE NOMINMAX=1
	PRIVATE WITH_IMGUI=1
    PRIVATE SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE ASSETS_ROOT="${CMAKE_BINARY_DIR}/assets"
    PRIVATE SPONZA_ROOT="${CMAKE_SOURCE_DIR}/assets/sponza-gltf-pbr")

if (MSVC)
	target_compile_definitions("${app_name}" PRIVATE SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
	target_compile_definitions("${app_name}" PRIVATE SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}")
endif()