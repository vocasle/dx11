set(current_target particle-system)

set(source_files
	src/CubeMap.cpp
	src/Game.cpp
	src/ShadowMap.cpp
	src/dx11.cpp
	src/stb_image.cpp
	src/DynamicCubeMap.cpp
	src/ParticleSystem.cpp
	)

set(header_files
	"include/CubeMap.h"
	include/Game.h
	include/Resource.h
	"include/ShadowMap.h"
	"include/Timer.h"
	include/dx11.h
	include/framework.h
	"include/stb_image.h"
	include/targetver.h
	"include/DynamicCubeMap.h"
	"include/ParticleSystem.h"
	include/Image.h
	)

set(pixel_shader_files
	shader/LightPS.hlsl
	shader/PhongPS.hlsl
	shader/PixelShaderPS.hlsl
	shader/SkyPS.hlsl
	shader/ParticlePS.hlsl)

set(vertex_shader_files
	shader/SkyVS.hlsl
	shader/VertexShaderVS.hlsl
	shader/ParticleVS.hlsl)

set(shader_header_files
	shader/Sky.hlsli
	shader/LightingHelper.hlsli
	shader/Common.hlsli
	shader/Particle.hlsli)

set(shader_source_files ${pixel_shader_files} ${vertex_shader_files})

add_executable(${current_target} WIN32 ${source_files} ${header_files} ${pixel_shader_files} ${vertex_shader_files} ${shader_header_files})

target_link_libraries(${current_target} d3d11.lib dxgi.lib dxguid.lib imgui 
	ne_utils
	ne_objloader
	ne_common
	ne_math
	ne_input)

target_compile_definitions(${current_target} PRIVATE ASSETS_ROOT="${CMAKE_BINARY_DIR}/assets")
target_compile_definitions(${current_target} PRIVATE SRC_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(${current_target} PRIVATE SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}/shaders")


target_include_directories(${current_target} PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(${current_target} PUBLIC 
	NOMINMAX=1
	WITH_IMGUI=1)

compile_shaders(${current_target} ${shader_source_files})