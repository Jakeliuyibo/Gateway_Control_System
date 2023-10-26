cmake_minimum_required(VERSION 3.12)

project(configparser)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

# 搜索头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 调用其他模块
find_package(jsoncpp REQUIRED)   # 查找json库
find_package(boost_filesystem REQUIRED)


set(LIBRARY_OUTPUT_PATH ${LIBPATH})
add_library(configparser STATIC ${SRC})
target_link_libraries(configparser jsoncpp boost_filesystem)  # 链接jsoncpp\config++库到你的可执行文件

