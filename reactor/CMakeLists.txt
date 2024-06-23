
# 生成source动态库，并进行链接
BuildModuleLibrary(
    reactor source SHARED

    isAutoFindSource                FALSE
    sourceList                      source.cpp
    incList                         source.hpp

    isLinkInc                       TRUE
    linkIncType                     PUBLIC
    linkIncList                     ${PROJECT_DIR}/utility/configparser ${PROJECT_DIR}/utility/logger ${PROJECT_DIR}/utility/rabbitmqclient

    isLinkLib                       TRUE
    linkLibType                     PUBLIC

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            configparser logger rabbitmqclient

    isLink3rdPartyLib               FALSE
    link3rdPartyLibsDir             
    link3rdPartyLibNeedFindPkgList  
    link3rdPartyLibNameList         
)

# 生成event动态库，并进行链接
BuildModuleLibrary(
    reactor event SHARED

    isAutoFindSource                FALSE
    sourceList                      event.cpp
    incList                         event.hpp

    isLinkInc                       TRUE
    linkIncType                     PUBLIC
    linkIncList                     /usr/include/jsoncpp ${PROJECT_DIR}/utility/logger

    isLinkLib                       TRUE
    linkLibType                     PUBLIC

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            logger

    isLink3rdPartyLib               TRUE
    link3rdPartyLibsDir             /usr/lib/x86_64-linux-gnu
    link3rdPartyLibNeedFindPkgList  jsoncpp
    link3rdPartyLibNameList         jsoncpp.so
)

# 生成processor动态库，并进行链接
BuildModuleLibrary(
    reactor processor SHARED

    isAutoFindSource                FALSE
    sourceList                      processor.cpp
    incList                         processor.hpp

    isLinkInc                       TRUE
    linkIncType                     PUBLIC
    linkIncList                     ${PROJECT_DIR}/utility/configparser ${PROJECT_DIR}/utility/threadpool

    isLinkLib                       TRUE
    linkLibType                     PUBLIC

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            configparser threadpool

    isLink3rdPartyLib               FALSE
    link3rdPartyLibsDir             
    link3rdPartyLibNeedFindPkgList  
    link3rdPartyLibNameList         
)

# 生成reactor动态库，并进行链接
BuildModuleLibrary(
    reactor reactor SHARED

    isAutoFindSource                FALSE
    sourceList                      reactor.cpp
    incList                         reactor.hpp

    isLinkInc                       TRUE
    linkIncType                     PUBLIC
    linkIncList                     ${PROJECT_DIR}/utility/singleton ${PROJECT_DIR}/utility/configparser ${PROJECT_DIR}/utility/logger ${PROJECT_DIR}/driver/commdevice ${PROJECT_DIR}/reactor 

    isLinkLib                       TRUE
    linkLibType                     PUBLIC

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            singleton logger configparser source event processor commdevice

    isLink3rdPartyLib               FALSE
    link3rdPartyLibsDir             
    link3rdPartyLibNeedFindPkgList  
    link3rdPartyLibNameList         
)
