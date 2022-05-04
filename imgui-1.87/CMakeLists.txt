set(source_files
	#backends/imgui_impl_allegro5.cpp
	#backends/imgui_impl_android.cpp
	#backends/imgui_impl_dx10.cpp
	backends/imgui_impl_dx11.cpp
	#backends/imgui_impl_dx12.cpp
	#backends/imgui_impl_dx9.cpp
	#backends/imgui_impl_glfw.cpp
	#backends/imgui_impl_glut.cpp
	#backends/imgui_impl_opengl2.cpp
	#backends/imgui_impl_opengl3.cpp
	#backends/imgui_impl_sdl.cpp
	#backends/imgui_impl_sdlrenderer.cpp
	#backends/imgui_impl_vulkan.cpp
	#backends/imgui_impl_wgpu.cpp
	backends/imgui_impl_win32.cpp
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_tables.cpp
	imgui_widgets.cpp
	misc/cpp/imgui_stdlib.cpp
	misc/fonts/binary_to_compressed_c.cpp
	#misc/freetype/imgui_freetype.cpp
)

set(header_files
	#backends/imgui_impl_allegro5.h
	#backends/imgui_impl_android.h
	#backends/imgui_impl_dx10.h
	backends/imgui_impl_dx11.h
	#backends/imgui_impl_dx12.h
	#backends/imgui_impl_dx9.h
	#backends/imgui_impl_glfw.h
	#backends/imgui_impl_glut.h
	#backends/imgui_impl_metal.h
	#backends/imgui_impl_opengl2.h
	#backends/imgui_impl_opengl3.h
	#backends/imgui_impl_opengl3_loader.h
	#backends/imgui_impl_osx.h
	#backends/imgui_impl_sdl.h
	#backends/imgui_impl_sdlrenderer.h
	#backends/imgui_impl_vulkan.h
	#backends/imgui_impl_wgpu.h
	backends/imgui_impl_win32.h
	imconfig.h
	imgui.h
	imgui_internal.h
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
	misc/cpp/imgui_stdlib.h
	#misc/freetype/imgui_freetype.h
	misc/single_file/imgui_single_file.h)

add_library(imgui ${source_files} ${header_files})

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/backends")

set_target_properties(imgui PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
