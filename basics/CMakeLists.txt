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

add_executable(${current_target} WIN32 ${source_files} ${header_files} ${pixel_shader_files} ${vertex_shader_files} ${shader_header_files})

set_property(TARGET ${current_target} PROPERTY CXX_STANDARD 20)

target_link_libraries(${current_target} d3d11.lib dxgi.lib dxguid.lib imgui ne_math ne_objloader)

target_compile_definitions(${current_target} PUBLIC
	NOMINMAX=1
	WITH_IMGUI=1)

configure_file(BasicsConfig.h.in BasicsConfig.h)
target_include_directories(${current_target} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# Build HLSL shaders
	add_custom_target(shaders)

	set(HLSL_SHADER_FILES ${vertex_shader_files} ${pixel_shader_files})

	set_source_files_properties(${vertex_shader_files} PROPERTIES ShaderType "vs")
	set_source_files_properties(${pixel_shader_files} PROPERTIES ShaderType "ps")
	set_source_files_properties(${HLSL_SHADER_FILES} PROPERTIES ShaderModel "4_0")

	foreach(FILE ${HLSL_SHADER_FILES})
		  get_filename_component(FILE_WE ${FILE} NAME_WE)
		  get_source_file_property(shadertype ${FILE} ShaderType)
		  get_source_file_property(shadermodel ${FILE} ShaderModel)
		  add_custom_command(TARGET shaders
			  COMMAND ${FXC_EXE} /nologo /Emain /T${shadertype}_${shadermodel} $<IF:$<CONFIG:DEBUG>,/Od,/O1> /Zi /Fo ${CMAKE_CURRENT_BINARY_DIR}/${FILE_WE}.cso /Fd ${CMAKE_CURRENT_BINARY_DIR}/${FILE_WE}.pdb ${FILE}
				     MAIN_DEPENDENCY ${FILE}
				     COMMENT "HLSL ${FILE}"
				     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				     VERBATIM)
	endforeach(FILE)

	add_dependencies(${current_target} shaders)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
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
endif()
