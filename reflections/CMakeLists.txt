set(current_target reflections)

set(sources
	Actor.cpp
	Camera.cpp
	CubeMap.cpp
	DeviceResources.cpp
	Game.cpp
	InputLayout.cpp
	Keyboard.cpp
	Math.cpp
	MeshGenerator.cpp
	Mouse.cpp
	Renderer.cpp
	ShadowMap.cpp
	Utils.cpp
	dx11.cpp
	objloader.cpp
	stb_image.cpp
	DynamicCubeMap.cpp
	)

set(headers
	Actor.h
	Camera.h
	CubeMap.h
	DeviceResources.h
	Game.h
	InputLayout.h
	Keyboard.h
	LightHelper.h
	Math.h
	MeshGenerator.h
	Mouse.h
	Renderer.h
	Resource.h
	ShadowMap.h
	Timer.h
	Utils.h
	dx11.h
	framework.h
	objloader.h
	stb_image.h
	targetver.h
	DynamicCubeMap.h
	)

set(pixel_shaders
	LightPS.hlsl
	PhongPS.hlsl
	PixelShaderPS.hlsl
	SkyPS.hlsl)

set(vertex_shaders
	SkyVS.hlsl
	VertexShaderVS.hlsl)

set(shader_headers 
	Sky.hlsli
	LightingHelper.hlsli
	Common.hlsli)

set(shader_source_files ${pixel_shaders} ${vertex_shaders})

add_executable(${current_target} WIN32 ${sources} ${headers} ${pixel_shaders} ${vertex_shaders} ${shader_headers})

target_link_libraries(${current_target} d3d11.lib dxgi.lib dxguid.lib imgui)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE "Debug")
endif()

target_compile_definitions(${current_target} PUBLIC
	ASSETS_ROOT="${assets_root}"
	NOMINMAX=1
	WITH_IMGUI=1
	SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}/shaders")

compile_shaders(${current_target} ${shader_source_files})