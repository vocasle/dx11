set(current_target particle-system)

set(source_files
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
	ParticleSystem.cpp
	)

set(header_files
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
	ParticleSystem.h
	)

set(pixel_shader_files
	LightPS.hlsl
	PhongPS.hlsl
	PixelShader.hlsl
	SkyPS.hlsl
	ParticlePS.hlsl)

set(vertex_shader_files
	SkyVS.hlsl
	VertexShader.hlsl
	ParticleVS.hlsl)

set(shader_header_files
	Sky.hlsli
	LightingHelper.hlsli
	Common.hlsli
	Particle.hlsli)

add_executable(${current_target} WIN32 ${source_files} ${header_files} ${pixel_shader_files} ${vertex_shader_files} ${shader_header_files})

target_link_libraries(${current_target} d3d11.lib dxgi.lib dxguid.lib imgui)

target_compile_definitions(${current_target} PUBLIC 
	ASSETS_ROOT="${assets_root}"
	NOMINMAX=1
	WITH_IMGUI=1
	SHADERS_ROOT="${CMAKE_CURRENT_BINARY_DIR}/Debug") # temporary hack, will fail on other build configurations

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

set_property(TARGET ${current_target} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
