set(current_target ne_common)

set(source_files
	"src/Camera.cpp"
	"src/DeviceResources.cpp"
	"src/Renderer.cpp"
	"src/ShaderManager.cpp"
	"src/InputLayout.cpp"
	"src/Actor.cpp"
	"src/MeshGenerator.cpp"
	"src/DynamicConstBuffer.cpp"
	"src/VertexBuffer.cpp"
	"src/Texture.cpp")
set(header_files
	"include/Camera.h"
	"include/DeviceResources.h"
	"include/Renderer.h"
	"include/ShaderManager.h"
	"include/LightHelper.h"
	"include/InputLayout.h"
	"include/Actor.h"
	"include/MeshGenerator.h"
	"include/DynamicConstBuffer.h"
	"include/Texture.h")

add_library(${current_target} ${source_files} ${header_files})

target_link_libraries(${current_target} ne_math ne_input ne_utils ne_objloader imgui)

target_include_directories(${current_target} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(${current_target} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

target_compile_definitions(${current_target} PUBLIC
	NOMINMAX=1
	WITH_IMGUI=1)
