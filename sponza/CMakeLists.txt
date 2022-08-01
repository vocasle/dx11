set(app_name "sponza")

set(app_source_files
    "src/main.cpp"
    "src/ModelLoader.cpp"
    "src/SponzaGame.cpp"
    "src/AssetManager.cpp")
set(app_header_files 
    "include/ModelLoader.h"
    "include/SponzaGame.h"
    "include/Buffer.h"
    "include/AssetManager.h")

add_executable("${app_name}" WIN32 "${app_source_files}" "${app_header_files}")

set(ASSIMP_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/assimp/lib")

target_include_directories("${app_name}" PRIVATE "include" "${CMAKE_SOURCE_DIR}/assimp/include")
target_link_directories("${app_name}" PRIVATE "${ASSIMP_LIBRARY_DIR}")

if (MSVC)
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
		set(CMAKE_BUILD_TYPE "Debug")
	endif()
	set(app_binary_dir ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()

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
    PRIVATE ASSETS_ROOT="${CMAKE_BINARY_DIR}/assets"
    PRIVATE SPONZA_ROOT="D:/Source/glTF-Sample-Models/2.0/Sponza/glTF"
    PRIVATE SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
	PRIVATE MATH_TEST=1)

set(pixel_shader_files
	shader/PhongPS.hlsl)

set(vertex_shader_files
	shader/ColorVS.hlsl)

set(shader_header_files
	shader/Common.hlsli)


target_include_directories(${app_name} PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include)

set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_TYPE ps)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_MODEL 5_0)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_ENABLE_DEBUG true)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_DISABLE_OPTIMIZATIONS true)

set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_TYPE vs)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_MODEL 5_0)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_ENABLE_DEBUG true)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_DISABLE_OPTIMIZATIONS true)

# Build HLSL shaders
set (shader_target ${app_name}-compile-shaders)
add_custom_target(${shader_target})

set(HLSL_SHADER_FILES ${vertex_shader_files} ${pixel_shader_files})

foreach(FILE ${HLSL_SHADER_FILES})
      get_filename_component(FILE_WE ${FILE} NAME_WE)
      get_source_file_property(shadertype ${FILE} VS_SHADER_TYPE)
      get_source_file_property(shadermodel ${FILE} VS_SHADER_MODEL)
      add_custom_command(TARGET ${shader_target}
          COMMAND ${FXC_EXE} /nologo
                    /WX
                    /Emain
                    /T${shadertype}_${shadermodel}
                    $<IF:$<CONFIG:DEBUG>,/Od,/O1>
                    /Zi
                    /Fo ${app_binary_dir}/${FILE_WE}.cso
                    /Fd ${app_binary_dir}/${FILE_WE}.pdb
                    ${FILE}
                 MAIN_DEPENDENCY ${FILE}
                 COMMENT "HLSL ${FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                 VERBATIM)
endforeach(FILE)

add_dependencies(${app_name} ${shader_target})