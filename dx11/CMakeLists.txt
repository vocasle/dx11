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

add_executable(dx11_plain_c WIN32 ${C_SOURCES} ${C_HEADERS})

target_link_libraries(dx11_plain_c d3d11.lib dxgi.lib dxguid.lib)
