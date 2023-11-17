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

# 生成queue静态库，并进行链接
add_library(source SHARED ${SRC})
link_directories(${LIBPATH})
target_link_libraries(source logger configparser rabbitmqclient)

# 生成event静态库，并进行链接
add_library(event SHARED ${SRC})
link_directories(${LIBPATH})
find_package(jsoncpp REQUIRED)
target_link_libraries(event logger jsoncpp)

# 生成processor静态库，并进行链接
add_library(processor SHARED ${SRC})
link_directories(${LIBPATH})
target_link_libraries(processor logger configparser threadpool)

# 生成reactor静态库，并进行链接
add_library(reactor SHARED ${SRC})
link_directories(${LIBPATH})
target_link_libraries(reactor singleton logger configparser source event processor commdevice)
