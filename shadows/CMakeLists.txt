set(current_target shadows)

set(sources
	Actor.cpp
	Camera.cpp
	DeviceResources.cpp
	Game.cpp
	Keyboard.cpp
	Math.cpp
	MeshGenerator.cpp
	Mouse.cpp
	Renderer.cpp
	ShadowMap.cpp
	Utils.cpp
	dx11.cpp
	objloader.cpp
	stb_image.cpp)

set(headers
	Actor.h
	Camera.h
	DeviceResources.h
	Game.h
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
	targetver.h)

set(pixel_shaders
	LightPS.hlsl
	PhongPS.hlsl
	PixelShaderPS.hlsl)

set(vertex_shaders
	VertexShaderVS.hlsl)

set(shader_headers 
	LightingHelper.hlsli
	Common.hlsli)

set(shader_source_files ${pixel_shaders} ${vertex_shaders})

add_executable(${current_target} WIN32 ${sources} ${headers} ${pixel_shaders} ${vertex_shaders} ${shader_headers})

target_link_libraries(${current_target} d3d11.lib dxgi.lib dxguid.lib)

target_compile_definitions(${current_target} PUBLIC 
	ASSETS_ROOT="${assets_root}"
	NOMINMAX=1
	SHADER_BLOB_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders")

compile_shaders(${current_target} ${shader_source_files})