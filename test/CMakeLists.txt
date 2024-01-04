cmake_minimum_required(VERSION 3.12)

# 本地引用
include_directories(${HEADPATH})
link_directories(${LIBPATH})
# link_libraries(systime logger configparser rwqueue safecontainer threadpool source reactor event filetransfer commdevice amqpclient)

# 设置输出路径并生成可执行文件
set(EXECUTABLE_OUTPUT_PATH ${EXECPATH})

add_executable(exec_management exec_management.cpp)
target_link_libraries(exec_management systime logger configparser)

add_executable(device_control device_control.cpp)
target_link_libraries(device_control systime logger configparser reactor event filetransfer)

add_executable(pressure_test pressure_test.cpp)
target_link_libraries(pressure_test systime logger configparser reactor event rabbitmqclient)



