cmake_minimum_required(VERSION 3.12)

# 本地引用
include_directories(${HEADPATH})
link_directories(${LIBPATH})
# link_libraries(systime logger configparser rwqueue safequeue threadpool source reactor event filetransfer commdevice amqpclient)

# 设置输出路径并生成可执行文件
set(EXECUTABLE_OUTPUT_PATH ${EXECPATH})

add_executable(test_reactor test_reactor.cpp)
target_link_libraries(test_reactor systime logger configparser reactor event filetransfer)

add_executable(test_rabbitmq test_rabbitmq.cpp)
target_link_libraries(test_rabbitmq systime logger configparser source reactor event)