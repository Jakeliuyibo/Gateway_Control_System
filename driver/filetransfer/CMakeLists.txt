cmake_minimum_required(VERSION 3.12)

# 搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

# 搜索头文件
include_directories(
	${PROJECT_DIR}/utility/singleton
    ${PROJECT_DIR}/utility/logger
	${PROJECT_DIR}/utility/safecontainer
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# 生成动态库
set(LIBRARY_OUTPUT_PATH ${LIBPATH})
add_library(filetransfer SHARED ${SRC})

# 链接本地库
link_directories(${LIBPATH})
target_link_libraries(filetransfer logger safecontainer)

# 链接第三方库
find_package(boost_system REQUIRED)
find_package(boost_filesystem REQUIRED)
target_link_libraries(filetransfer boost_system.so boost_filesystem.so)
