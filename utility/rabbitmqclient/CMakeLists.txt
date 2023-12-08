cmake_minimum_required(VERSION 3.12)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

set(RABBITMQ_PATH /usr/local)

# 搜索头文件
include_directories(
    ${RABBITMQ_PATH}/include
    ${PROJECT_DIR}/utility/singleton
    ${PROJECT_DIR}/utility/logger
    ${PROJECT_DIR}/utility/safequeue
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# 生成静态库
set(LIBRARY_OUTPUT_PATH ${LIBPATH})
add_library(rabbitmqclient SHARED ${SRC})

# 链接本地静态库
# link_directories(${RABBITMQ_PATH}/lib)
target_link_libraries(rabbitmqclient ${RABBITMQ_PATH}/lib/librabbitmq.so)

link_directories(${LIBPATH})
target_link_libraries(rabbitmqclient logger safequeue)