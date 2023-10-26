cmake_minimum_required(VERSION 3.12)

project(rabbitmqclient)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

set(RABBITMQ_PATH ${PROJECT_SOURCE_DIR}//thirdparty//rabbitmq)

# 搜索头文件
include_directories(
        ${HEADPATH}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${RABBITMQ_PATH}//include
    )
# 静态链接
link_directories(${RABBITMQ_PATH}//lib)
link_libraries(${RABBITMQ_PATH}//lib//librabbitmq.4.dll.a)
link_directories(${LIBPATH})
link_libraries(logger)

set(LIBRARY_OUTPUT_PATH ${LIBPATH})

add_library(rabbitmqclient STATIC ${SRC})
