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
	)

set(pixel_shaders
	LightPS.hlsl
	PhongPS.hlsl
	PixelShader.hlsl
	SkyPS.hlsl)

set(vertex_shaders
	SkyVS.hlsl
	VertexShader.hlsl)

set(shader_headers 
	Sky.hlsli
	LightingHelper.hlsli
	Common.hlsli)

add_executable(${current_target} WIN32 ${sources} ${headers} ${pixel_shaders} ${vertex_shaders} ${shader_headers})

target_link_libraries(${current_target} d3d11.lib dxgi.lib dxguid.lib imgui)

target_compile_definitions(${current_target} PUBLIC 
	ASSETS_ROOT="${assets_root}"
	NOMINMAX=1
	WITH_IMGUI=1)

set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_ENABLE_DEBUG true)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_DISABLE_OPTIMIZATIONS true)

set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_ENABLE_DEBUG true)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_DISABLE_OPTIMIZATIONS true)

set_property(TARGET ${current_target} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
