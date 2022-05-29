set(current_target objloader)

set(source_files objloader.cpp)
set(header_files objloader.h)

add_library(${current_target} ${source_files} ${header_files})

target_link_libraries(${current_target} ne_math)

target_include_directories(${current_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${current_target} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
