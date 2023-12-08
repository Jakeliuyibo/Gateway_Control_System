cmake_minimum_required(VERSION 3.12)
project(Middleware_development)

set(CMAKE_CXX_STANDARD 14)

# 工程目录
set(PROJECT_DIR ${PROJECT_SOURCE_DIR})

# 库生成的路径
set(LIBPATH ${PROJECT_SOURCE_DIR}/lib)

# 可执行文件的路径
set(EXECPATH ${PROJECT_SOURCE_DIR}/bin)

# 头文件的路径
set(HEADPATH
	${PROJECT_DIR}/utility/singleton
    ${PROJECT_DIR}/utility/configparser
	${PROJECT_DIR}/utility/systime
	${PROJECT_DIR}/utility/safequeue
    ${PROJECT_DIR}/utility/logger
    ${PROJECT_DIR}/utility/rabbitmqclient
	${PROJECT_DIR}/utility/threadpool
	${PROJECT_DIR}/driver/filetransfer
	${PROJECT_DIR}/driver/commdevice
	${PROJECT_DIR}/reactor
	)

# 添加子目录
add_subdirectory(thirdparty)
add_subdirectory(utility)
add_subdirectory(driver)
add_subdirectory(reactor)
add_subdirectory(test)
