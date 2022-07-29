set(app_name "sponza")

set(app_source_files "src/main.cpp")
set(app_header_files "include/main.h")

add_executable("${app_name}" "${app_source_files}" "${app_header_files}")

target_include_directories("${app_name}" PRIVATE "include")