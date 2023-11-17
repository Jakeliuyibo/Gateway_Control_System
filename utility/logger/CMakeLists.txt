cmake_minimum_required(VERSION 3.12)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

# 搜索头文件
include_directories(
    ${PROJECT_DIR}/utility/singleton
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# 生成静态库
set(LIBRARY_OUTPUT_PATH ${LIBPATH})
add_library(logger SHARED ${SRC})

# 链接本地静态库
link_directories(${LIBPATH})
target_link_libraries(logger singleton)

# 链接第三方静态库
find_package(spdlog REQUIRED)   # 查找spdlog库
find_package(fmt REQUIRED)      # 查找fmt库
target_link_libraries(logger spdlog fmt)  # 链接spdlog库到你的可执行文件
