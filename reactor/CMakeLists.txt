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
add_library(queue STATIC ${SRC})
link_directories(${LIBPATH})
target_link_libraries(queue logger configparser rabbitmqclient)

# 生成event静态库，并进行链接
add_library(event STATIC ${SRC})
link_directories(${LIBPATH})
find_package(jsoncpp REQUIRED)
target_link_libraries(event logger jsoncpp)

# 生成queue静态库，并进行链接
add_library(processor STATIC ${SRC})
link_directories(${LIBPATH})
target_link_libraries(processor logger configparser threadpool)
