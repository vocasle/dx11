add_executable(ne_test main.cpp)
target_link_libraries(ne_test ne_common ne_math ne_utils dxgi.lib d3d11.lib dxguid.lib)