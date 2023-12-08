cmake_minimum_required(VERSION 3.12)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

set(RABBITMQ_PATH /usr/local)

# 搜索头文件
include_directories(
    ${RABBITMQ_PATH}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

# 构建AMQP
set(LIBRARY_OUTPUT_PATH ${LIBPATH})
add_library(amqpcpp SHARED ${SRC})

target_link_libraries(amqpcpp ${RABBITMQ_PATH}/lib/librabbitmq.so)

