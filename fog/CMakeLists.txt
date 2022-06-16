set(current_target fog)

message(STATUS "Current build directory: ${CMAKE_CURRENT_BINARY_DIR}")

set(source_files
	"src/fog.cpp"
	  "src/FogGame.cpp")

set(header_files
	"include/fog.h"   "include/FogGame.h")

set(pixel_shader_files
	)

set(vertex_shader_files
	)

set(shader_header_files
	)

add_executable(${current_target} WIN32 ${source_files} ${header_files} ${pixel_shader_files} ${vertex_shader_files} ${shader_header_files})

target_link_libraries(${current_target} d3d11.lib dxgi.lib dxguid.lib imgui ne_math ne_objloader ne_utils ne_input ne_common)

target_compile_definitions(${current_target} PUBLIC
	NOMINMAX=1
	WITH_IMGUI=1)

configure_file("include/Config.h.in" Config.h)

target_include_directories(${current_target} PRIVATE 
	${CMAKE_CURRENT_BINARY_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR}/include)

set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_TYPE ps)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_MODEL 5_0)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_ENABLE_DEBUG true)
set_property(SOURCE ${pixel_shader_files} PROPERTY VS_SHADER_DISABLE_OPTIMIZATIONS true)

set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_TYPE vs)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_MODEL 5_0)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_ENABLE_DEBUG true)
set_property(SOURCE ${vertex_shader_files} PROPERTY VS_SHADER_DISABLE_OPTIMIZATIONS true)

set_property(TARGET ${current_target} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(FXC_EXE "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.19041.0\\x64\\fxc.exe")
	# Build HLSL shaders
	set (shader_target ${current_target}-compile-shaders)
	add_custom_target(${shader_target})

	set(HLSL_SHADER_FILES ${vertex_shader_files} ${pixel_shader_files})

	foreach(FILE ${HLSL_SHADER_FILES})
		  get_filename_component(FILE_WE ${FILE} NAME_WE)
		  get_source_file_property(shadertype ${FILE} VS_SHADER_TYPE)
		  get_source_file_property(shadermodel ${FILE} VS_SHADER_MODEL)
		  add_custom_command(TARGET ${shader_target}
			  COMMAND ${FXC_EXE} /nologo
						/Fh ${CMAKE_CURRENT_BINARY_DIR}/${FILE_WE}.h
						/Emain
						/T${shadertype}_${shadermodel}
						$<IF:$<CONFIG:DEBUG>,/Od,/O1>
						/Zi
						/Fo ${CMAKE_CURRENT_BINARY_DIR}/${FILE_WE}.cso
						/Fd ${CMAKE_CURRENT_BINARY_DIR}/${FILE_WE}.pdb
						${FILE}
				     MAIN_DEPENDENCY ${FILE}
				     COMMENT "HLSL ${FILE}"
				     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				     VERBATIM)
	endforeach(FILE)

	add_dependencies(${current_target} ${shader_target})
endif()
