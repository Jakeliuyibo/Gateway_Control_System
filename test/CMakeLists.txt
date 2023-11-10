cmake_minimum_required(VERSION 3.12)

# 本地引用
include_directories(${HEADPATH})
link_directories(${LIBPATH})
link_libraries(systime logger configparser reactor serial tcpserver)

# # 第三方库引用
# include_directories(/usr/include)
# link_directories(/usr/lib/x86_64-linux-gnu)
# link_libraries(uv)

# 设置输出路径并生成可执行文件
set(EXECUTABLE_OUTPUT_PATH ${EXECPATH})

add_executable(test_reactor test_reactor.cpp)
add_executable(test_driver  test_driver.cpp)