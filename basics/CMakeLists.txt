set(current_target basics)

set(source_files
	DeviceResources.cpp
	Game.cpp
	Keyboard.cpp
	Mouse.cpp
	Renderer.cpp
	Utils.cpp
	dx11.cpp
	stb_image.cpp
	)

set(header_files
	DeviceResources.h
	Game.h
	Keyboard.h
	Mouse.h
	Renderer.h
	Timer.h
	Utils.h
	dx11.h
	framework.h
	stb_image.h
	targetver.h
	)

set(pixel_shader_files
	BasicPS.hlsl
	)

set(vertex_shader_files
	BasicVS.hlsl
	)

set(shader_header_files
	)

set(shader_source_files ${pixel_shader_files} ${vertex_shader_files})

add_executable(${current_target} WIN32 ${source_files} ${header_files} ${pixel_shader_files} ${vertex_shader_files} ${shader_header_files})

set_property(TARGET ${current_target} PROPERTY CXX_STANDARD 20)

target_link_libraries(${current_target} d3d11.lib dxgi.lib dxguid.lib imgui ne_math ne_objloader)

target_compile_definitions(${current_target} PUBLIC
	NOMINMAX=1
	WITH_IMGUI=1)

configure_file(BasicsConfig.h.in BasicsConfig.h)
target_include_directories(${current_target} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
compile_shaders(${current_target} ${shader_source_files})
