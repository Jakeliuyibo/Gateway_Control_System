cmake_minimum_required(VERSION 3.12)

# 本地引用
include_directories(${HEADPATH})
link_directories(${LIBPATH})
link_libraries(systime logger configparser rwqueue safequeue threadpool reactor event filetransfer commdevice)

# 设置输出路径并生成可执行文件
set(EXECUTABLE_OUTPUT_PATH ${EXECPATH})

add_executable(test_reactor test_reactor.cpp)
