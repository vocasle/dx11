set(current_target ne_common)

set(source_files 
	"Game.cpp" 
	"Camera.cpp" 
	"DeviceResources.cpp"
	"Renderer.cpp"
	"ShaderManager.cpp"
	"InputLayout.cpp"
	"Actor.cpp"
	"MeshGenerator.cpp" "DynamicConstBuffer.cpp")
set(header_files 
	"Game.h"
	"Camera.h"
	"DeviceResources.h"
	"Renderer.h" 
	"ShaderManager.h" 
	"LightHelper.h"
	"InputLayout.h"
	"Actor.h"
	"MeshGenerator.h"
	"DynamicConstBuffer.h")

add_library(${current_target} ${source_files} ${header_files})

target_link_libraries(${current_target} ne_math ne_input ne_utils ne_objloader imgui)

target_include_directories(${current_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${current_target} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

target_compile_definitions(${current_target} PUBLIC
	NOMINMAX=1
	WITH_IMGUI=1)
