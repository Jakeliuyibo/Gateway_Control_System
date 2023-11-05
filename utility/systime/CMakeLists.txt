cmake_minimum_required(VERSION 3.12)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

# 搜索头文件
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# 生成静态库
set(LIBRARY_OUTPUT_PATH ${LIBPATH})
add_library(systime STATIC ${SRC})