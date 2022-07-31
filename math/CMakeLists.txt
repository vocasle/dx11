set(current_target ne_math)

set(source_files NE_Math.cpp)
set(header_files NE_Math.h)

add_library(${current_target} ${source_files} ${header_files})

target_include_directories(${current_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(${current_target} PRIVATE MATH_TEST=1)

set_target_properties(${current_target} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
