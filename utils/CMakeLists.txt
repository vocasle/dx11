add_library(utils "Utils.h" "Utils.cpp" "Image.h" "stb_image.h" "stb_image.cpp")
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})