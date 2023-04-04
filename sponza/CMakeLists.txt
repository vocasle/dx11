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

add_custom_command(TARGET "${app_name}"
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${ASSIMP_LIBRARY_DIR}/assimp-vc143-mt.dll" "${CMAKE_CURRENT_BINARY_DIR}/.")

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
        PRIVATE SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}/shaders"
        PRIVATE MATH_TEST=1)

target_include_directories(${app_name} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_FLAGS "/Fd \"$(OutDir)%(Filename).pdb\"")
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_ENABLE_DEBUG true)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_DISABLE_OPTIMIZATIONS true)

set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_FLAGS "/Fd \"$(OutDir)%(Filename).pdb\"")
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_ENABLE_DEBUG true)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_DISABLE_OPTIMIZATIONS true)

set(shader_source_files ${pixel_shader_files} ${vertex_shader_files})

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

compile_shaders(${app_name} ${shader_source_files})