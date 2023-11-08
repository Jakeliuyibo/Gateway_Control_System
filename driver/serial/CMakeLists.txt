cmake_minimum_required(VERSION 3.12)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

# 搜索头文件
include_directories(
    ${HEADPATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# 设置静态库路径
set(LIBRARY_OUTPUT_PATH ${LIBPATH})
add_library(serial STATIC ${SRC})

# 链接本地静态库
link_directories(${LIBPATH})
target_link_libraries(serial logger configparser)

# 链接第三方静态库
find_package(boost_system REQUIRED)
target_link_libraries(serial boost_system)