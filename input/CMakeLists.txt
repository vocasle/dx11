set(current_target ne_input)

set(source_files "Keyboard.cpp" "Mouse.cpp")
set(header_files "Keyboard.h" "Mouse.h")

add_library(${current_target} ${source_files} ${header_files})

target_link_libraries(${current_target} ne_math ne_utils)

target_include_directories(${current_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${current_target} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")