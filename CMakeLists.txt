cmake_minimum_required(VERSION 3.22)

project(Middleware_development 
	VERSION 3.0
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# 库目录和可执行程序目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/share)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/install)
message(" * 静态库路径    : ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message(" * 动态库路径    : ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(" * 可执行路径    : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(" * 项目的路径    : ${PROJECT_DIR}")
message(" * 安装的路径    : ${CMAKE_INSTALL_PREFIX}")

# 使能参数解析器
include(CMakeParseArguments)

#############################################################################
# 函数：依赖其他文件的(静态/动态)库编译
# 参数说明:
#   1、_folderName: 文件夹名 _libName: 库名 _buildType: 编译类型(STATIC/SHARED)
#   2、isAutoFindSource(bool): 自动搜索编译文件
#       i、sourceList(one-value)：源文件列表
#       ii、incList(one-value)：头文件列表
#   3、isLinkInc(bool): 链接头文件
#       i、linkIncType(one-value): 链接类型(PRIVATE/PUBLIC/INTERFACE)
#       ii、linkIncList(multi-value): 链接头文件列表
#   4、isLinkLib(bool): 链接库
#       i、linkLibType(one-value): 链接类型(PRIVATE/PUBLIC/INTERFACE)
#       ii、isLinkLocalLib(bool): 链接本地库
#           a.linkLocalLibDir(one-value): 路径
#           b.linkLocalLibNameList(multi-value): 链接库文件列表
#       iii、isLink3rdPartyLib(bool): 链接三方库
#           a.link3rdPartyLibDir(one-value): 路径
#           b.link3rdPartyLibNeedFindPkgList(multi-value): 需要find_package的库文件列表
#           c.link3rdPartyLibNameList(multi-value): 链接库文件列表
#############################################################################
function(BuildModuleLibrary _folderName _libName _buildType)
    set(options)
    set(boolArgs isAutoFindSource isLinkInc isLinkLib isLinkLocalLib isLink3rdPartyLib)
    set(oneValueArgs sourceList incList linkIncType linkLibType linkLocalLibDir link3rdPartyLibsDir linkIncList linkLocalLibNameList link3rdPartyLibNeedFindPkgList link3rdPartyLibNameList)
    set(multiValueArgs)
    cmake_parse_arguments("" "${options}" "${boolArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message(" * 执行${_folderName}文件夹下的CMakelists.txt")

    # 1、自动搜索文件
    if(${_isAutoFindSource})
        message("  -> 自动搜索源文件和头文件")
        # 搜索所用文件
        file(GLOB _sources "*.cpp")
        file(GLOB _includes "*.hpp")
    else()
        message("  -> 指定源文件和头文件")
        set(_sources ${_sourceList})
        set(_includes ${_incList})
    endif()
    message("    -> 搜索到源文件${_sources},头文件${_includes}")

    # 2、生成库
    if(NOT _buildType)
        message("  -> 没有传递编译库类型，默认设置为STATIC")
        set(_buildType "STATIC")
    endif()

    add_library(${_libName} ${_buildType} "")
    target_sources(${_libName}
    PRIVATE
        ${_sources}
    PUBLIC
        ${_includes}
    )
    message("  -> 以${_buildType}类型编译${_libName}库")

    # 3、链接头文件
    if(${_isLinkInc})
        message("  -> 使能链接头文件,权限类型为${_linkIncType}")
        target_include_directories(${_libName}
            ${_linkIncType}
            ${_linkIncList}
        )
        message("    -> 链接头文件列表${_linkIncList}")
    else()
        message("  -> 不需要链接头文件")
    endif()

    # 4、链接库
    if(${_isLinkLib})
        message("  -> 使能链接库,权限类型为${_linkLibType}")

        # i、本地库
        if(${_isLinkLocalLib})
            message("    -> 链接${_linkLocalLibDir}目录下的${_linkLocalLibNameList}库")
            link_directories(${_linkLocalLibDir})
            target_link_libraries(${_libName} 
                ${_linkLibType}
                ${_linkLocalLibNameList}
            )
        else()
            message("    -> 不需要链接本地库")
        endif()

        # ii、三方库
        if(${_isLink3rdPartyLib})
            message("    -> 链接${_link3rdPartyLibsDir}目录及通过寻址器发现${_link3rdPartyLibNeedFindPkgList}配置下的${_link3rdPartyLibNameList}库")
            link_directories(${_link3rdPartyLibsDir})
            foreach(_lib ${_link3rdPartyLibNeedFindPkgList})
                find_package(${_lib} REQUIRED)
            endforeach()
            target_link_libraries(${_libName} 
                ${_linkLibType}
                ${_link3rdPartyLibNameList}
            )
        else()
            message("    -> 不需要链接三方库")
        endif()

    else()
        message("  -> 不需要链接库")
    endif()

endfunction()

#############################################################################
# 函数：可执行程序编译
# 参数说明:
#   1、_folderName: 文件夹名 _libName: 库名 _buildType: 编译类型(STATIC/SHARED)
#   2、isAutoFindSource(bool): 自动搜索编译文件
#       i、sourceList(one-value)：源文件列表
#       ii、incList(one-value)：头文件列表
#   3、isLinkInc(bool): 链接头文件
#       i、linkIncType(one-value): 链接类型(PRIVATE/PUBLIC/INTERFACE)
#       ii、linkIncList(multi-value): 链接头文件列表
#   4、isLinkLib(bool): 链接库
#       i、linkLibType(one-value): 链接类型(PRIVATE/PUBLIC/INTERFACE)
#       ii、isLinkLocalLib(bool): 链接本地库
#           a.linkLocalLibDir(one-value): 路径
#           b.linkLocalLibNameList(multi-value): 链接库文件列表
#       iii、isLink3rdPartyLib(bool): 链接三方库
#           a.link3rdPartyLibDir(one-value): 路径
#           b.link3rdPartyLibNeedFindPkgList(multi-value): 需要find_package的库文件列表
#           c.link3rdPartyLibNameList(multi-value): 链接库文件列表
#############################################################################
function(BuildModuleExec _folderName _libName)
    set(options)
    set(boolArgs isAutoFindSource isLinkInc isLinkLib isLinkLocalLib isLink3rdPartyLib)
    set(oneValueArgs sourceList incList linkIncType linkLibType linkLocalLibDir link3rdPartyLibsDir linkIncList linkLocalLibNameList link3rdPartyLibNeedFindPkgList link3rdPartyLibNameList)
    set(multiValueArgs)
    cmake_parse_arguments("" "${options}" "${boolArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message(" * 执行${_folderName}文件夹下的CMakelists.txt")

    # 1、自动搜索文件
    if(${_isAutoFindSource})
        message("  -> 自动搜索源文件和头文件")
        # 搜索所用文件
        file(GLOB _sources "*.cpp")
        file(GLOB _includes "*.hpp")
    else()
        message("  -> 指定源文件和头文件")
        set(_sources ${_sourceList})
        set(_includes ${_incList})
    endif()
    message("    -> 搜索到源文件${_sources},头文件${_includes}")

    # 2、生成库
    add_executable(${_libName} "")
    target_sources(${_libName}
    PRIVATE
        ${_sources}
    PUBLIC
        ${_includes}
    )
    message("  -> 编译${_libName}可执行程序")

    # 3、链接头文件
    if(${_isLinkInc})
        message("  -> 使能链接头文件,权限类型为${_linkIncType}")
        target_include_directories(${_libName}
            ${_linkIncType}
            ${_linkIncList}
        )
        message("    -> 链接头文件列表${_linkIncList}")
    else()
        message("  -> 不需要链接头文件")
    endif()

    # 4、链接库
    if(${_isLinkLib})
        message("  -> 使能链接库,权限类型为${_linkLibType}")

        # i、本地库
        if(${_isLinkLocalLib})
            message("    -> 链接${_linkLocalLibDir}目录下的${_linkLocalLibNameList}库")
            link_directories(${_linkLocalLibDir})
            target_link_libraries(${_libName} 
                ${_linkLibType}
                ${_linkLocalLibNameList}
            )
        else()
            message("    -> 不需要链接本地库")
        endif()

        # ii、三方库
        if(${_isLink3rdPartyLib})
            message("    -> 链接${_link3rdPartyLibsDir}目录及通过寻址器发现${_link3rdPartyLibNeedFindPkgList}配置下的${_link3rdPartyLibNameList}库")
            link_directories(${_link3rdPartyLibsDir})
            foreach(_lib ${_link3rdPartyLibNeedFindPkgList})
                find_package(${_lib} REQUIRED)
            endforeach()
            target_link_libraries(${_libName} 
                ${_linkLibType}
                ${_link3rdPartyLibNameList}
            )
        else()
            message("    -> 不需要链接三方库")
        endif()

    else()
        message("  -> 不需要链接库")
    endif()

endfunction()

# 添加子目录
add_subdirectory(thirdparty)
add_subdirectory(utility)
add_subdirectory(driver)
add_subdirectory(reactor)

# 编译测试程序
option(build_tests "Build unit tests?" ON)
if(build_tests)
	add_subdirectory(test)
endif()
