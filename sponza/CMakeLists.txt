set(app_name "sponza")

set(app_source_files
        "src/main.cpp"
        "src/ModelLoader.cpp"
        "src/SponzaGame.cpp"
        "src/AssetManager.cpp"
        "src/ParticleSystem.cpp"
        "src/ShadowMap.cpp"
        "src/DynamicCubeMap.cpp"
        src/Actor.cpp)

set(app_header_files
        "include/ModelLoader.h"
        "include/SponzaGame.h"
        "include/Buffer.h"
        "include/AssetManager.h"
        "include/ParticleSystem.h"
        "include/ShadowMap.h"
        "include/DynamicCubeMap.h"
        include/Actor.h)

set(pixel_shader_files
        "shader/PhongPS.hlsl"
        "shader/ParticlePS.hlsl")

set(vertex_shader_files
        "shader/ColorVS.hlsl"
        "shader/ParticleVS.hlsl"
        "shader/ShadowVS.hlsl")

set(shader_header_files
        "shader/Common.hlsli"
        "shader/Particle.hlsli")

add_executable("${app_name}" WIN32 "${app_source_files}" "${app_header_files}" ${pixel_shader_files} ${vertex_shader_files} ${shader_header_files})

set(ASSIMP_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/assimp/lib")

target_include_directories("${app_name}" PRIVATE "include" "${CMAKE_SOURCE_DIR}/assimp/include")
target_link_directories("${app_name}" PRIVATE "${ASSIMP_LIBRARY_DIR}")

if (MSVC)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
        set(CMAKE_BUILD_TYPE "Debug")
    endif ()
    set(app_binary_dir ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif ()

add_custom_command(TARGET "${app_name}"
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${ASSIMP_LIBRARY_DIR}/assimp-vc143-mt.dll" "${app_binary_dir}/.")

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
        PRIVATE ASSETS_ROOT="${CMAKE_SOURCE_DIR}/assets"
        PRIVATE SPONZA_ROOT="${CMAKE_SOURCE_DIR}/assets/Sponza/glTF"
        PRIVATE SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
        PRIVATE MATH_TEST=1)

target_include_directories(${app_name} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_ENABLE_DEBUG true)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_DISABLE_OPTIMIZATIONS true)

set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_ENABLE_DEBUG true)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_DISABLE_OPTIMIZATIONS true)