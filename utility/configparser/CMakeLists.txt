cmake_minimum_required(VERSION 3.12)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

# 搜索头文件
include_directories(
    ${HEADPATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# 生成静态库
set(LIBRARY_OUTPUT_PATH ${LIBPATH})
add_library(configparser SHARED ${SRC})

# 链接本地静态库
link_directories(${LIBPATH})
target_link_libraries(configparser logger)

# 链接第三方静态库
find_package(boost_filesystem REQUIRED)
target_link_libraries(configparser boost_filesystem.so)
target_compile_definitions(configparser PRIVATE BOOST_BIND_GLOBAL_PLACEHOLDERS)
