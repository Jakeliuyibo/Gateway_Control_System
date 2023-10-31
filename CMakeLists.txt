cmake_minimum_required(VERSION 3.12)
project(Middleware_development)

set(CMAKE_CXX_STANDARD 11)

# 定义变量
# 静态库生成的路径
set(LIBPATH ${PROJECT_SOURCE_DIR}/lib)

# 可执行文件的路径
set(EXECPATH ${PROJECT_SOURCE_DIR}/bin)

# 头文件的路径
set(HEADPATH
    ${PROJECT_SOURCE_DIR}//utility//logger
    ${PROJECT_SOURCE_DIR}//utility//configparser
	${PROJECT_SOURCE_DIR}//utility//systime
	${PROJECT_SOURCE_DIR}//utility//singleton
    # ${PROJECT_SOURCE_DIR}//utility//rabbitmqclient
    # ${PROJECT_SOURCE_DIR}//thirdparty//rabbitmq/include
	# ${PROJECT_SOURCE_DIR}//utility//reactor
	# ${PROJECT_SOURCE_DIR}//utility//threadpool
	)

# 添加子目录

add_subdirectory(utility)
add_subdirectory(driver)
add_subdirectory(app)
add_subdirectory(test)
