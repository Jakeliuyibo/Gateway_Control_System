cmake_minimum_required(VERSION 3.12)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

# 搜索头文件
include_directories(
    /usr/include/jsoncpp
    ${HEADPATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# 设置静态库路径
set(LIBRARY_OUTPUT_PATH ${LIBPATH})

# 链接本地库
link_directories(${LIBPATH})

# 生成queue动态库，并进行链接
add_library(source SHARED ${SRC})
target_link_libraries(source logger configparser rabbitmqclient)

# 生成event动态库，并进行链接
add_library(event SHARED ${SRC})
find_package(jsoncpp REQUIRED)
target_link_libraries(event logger jsoncpp.so)

# 生成processor动态库，并进行链接
add_library(processor SHARED ${SRC})
target_link_libraries(processor logger configparser threadpool)

# 生成reactor动态库，并进行链接
add_library(reactor SHARED ${SRC})
target_link_libraries(reactor singleton logger configparser source event processor commdevice)
