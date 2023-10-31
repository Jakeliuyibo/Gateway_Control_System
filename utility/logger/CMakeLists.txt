cmake_minimum_required(VERSION 3.12)

project(logger)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

# 搜索头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 调用其他模块
include_directories(${HEADPATH})
link_directories(${LIBPATH})
link_libraries(singleton)

# 生成静态库
set(LIBRARY_OUTPUT_PATH ${LIBPATH})
add_library(logger STATIC ${SRC})

find_package(spdlog REQUIRED)   # 查找spdlog库
find_package(fmt REQUIRED)      # 查找fmt库

target_link_libraries(logger spdlog fmt)  # 链接spdlog库到你的可执行文件
