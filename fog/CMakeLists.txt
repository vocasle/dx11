set(current_target fog)

message(STATUS "Current build directory: ${CMAKE_CURRENT_BINARY_DIR}")

set(source_files
	"src/CubeMap.cpp"
	"src/Game.cpp"
	"src/ShadowMap.cpp"
	"src/dx11.cpp"
	"src/DynamicCubeMap.cpp"
        )

set(header_files
	"include/CubeMap.h"
	"include/Game.h"
	"include/Resource.h"
	"include/ShadowMap.h"
	"include/dx11.h"
	"include/framework.h"
	"include/targetver.h"
	"include/DynamicCubeMap.h"
		)

set(pixel_shader_files
	shader/ShadowPS.hlsl
	shader/LightPS.hlsl
	shader/PhongPS.hlsl
	shader/DefaultPS.hlsl
	shader/SkyPS.hlsl
	shader/ParticlePS.hlsl
	shader/FogPS.hlsl)

set(vertex_shader_files
	shader/ShadowVS.hlsl
	shader/SkyVS.hlsl
	shader/ColorVS.hlsl
	shader/ParticleVS.hlsl
	shader/FogVS.hlsl)

set(shader_header_files
	shader/Sky.hlsli
	shader/LightingHelper.hlsli
	shader/Common.hlsli
	shader/Particle.hlsli
	shader/Fog.hlsli)

add_executable(${current_target} WIN32 ${source_files} ${header_files} ${pixel_shader_files} ${vertex_shader_files} ${shader_header_files})

target_link_libraries(${current_target}
	d3d11.lib
	dxgi.lib
	dxguid.lib
	imgui
	ne_math
	ne_objloader
	ne_utils
	ne_input
	ne_common)

target_compile_definitions(${current_target} PUBLIC
	NOMINMAX=1
	WITH_IMGUI=1)

target_include_directories(${current_target} PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(${current_target} PRIVATE ASSETS_ROOT="${CMAKE_BINARY_DIR}/assets")
target_compile_definitions(${current_target} PRIVATE SRC_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(${current_target} PRIVATE SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}/shaders")
	
target_include_directories(${current_target} PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include)
	
set(shader_source_files ${pixel_shader_files} ${vertex_shader_files})

compile_shaders(${current_target} ${shader_source_files})