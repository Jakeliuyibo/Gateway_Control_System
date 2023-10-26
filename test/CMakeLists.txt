cmake_minimum_required(VERSION 3.12)

# 引用头文件
include_directories(${HEADPATH})

# 链接静态库和动态库
link_directories(${LIBPATH})
link_libraries(systime singleton logger configparser)

# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${EXECPATH})

# 生成可执行文件
add_executable(test_logger test_logger.cpp)