set(C_SOURCES
	Camera.c
	DeviceResources.c
	Game.c
	Keyboard.c
	Math.c
	Mouse.c
	Renderer.c
	Utils.c
	dx11.c
	objloader.c
	stb_image.c)

set(C_HEADERS
	Camera.h
	DeviceResources.h
	Game.h
	Keyboard.h
	Math.h
	Mouse.h
	Renderer.h
	Resource.h
	Timer.h
	Utils.h
	dx11.h
	framework.h
	objloader.h
	stb_image.h
	targetver.h)

set(PIXEL_SHADERS
	LightPS.hlsl
	PhongPS.hlsl
	PixelShader.hlsl)

set(VERTEX_SHADERS
	VertexShader.hlsl
	LightingHelper.hlsli)

add_executable(dx11_plain_c WIN32 ${C_SOURCES} ${C_HEADERS} ${PIXEL_SHADERS} ${VERTEX_SHADERS})

target_link_libraries(dx11_plain_c d3d11.lib dxgi.lib dxguid.lib)

target_compile_definitions(dx11_plain_c PUBLIC ASSETS_ROOT="${assets_root}")

set_property(SOURCE ${PIXEL_SHADERS} PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${PIXEL_SHADERS} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${PIXEL_SHADERS} PROPERTY VS_SHADER_MODEL 4.0_level_9_3)
set_property(SOURCE ${PIXEL_SHADERS} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)

set_property(SOURCE ${VERTEX_SHADERS} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${VERTEX_SHADERS} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${VERTEX_SHADERS} PROPERTY VS_SHADER_MODEL 4.0_level_9_3)
set_property(SOURCE ${VERTEX_SHADERS} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)

set_property(TARGET dx11_plain_c PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
