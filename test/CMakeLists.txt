
# 生成 exec_management 可执行程序
BuildModuleExec(
    test exec_management

    isAutoFindSource                FALSE
    sourceList                      exec_management.cpp
    incList                         

    isLinkInc                       TRUE
    linkIncType                     PRIVATE
    linkIncList                     ${PROJECT_DIR}/utility/systime ${PROJECT_DIR}/utility/logger ${PROJECT_DIR}/utility/configparser 

    isLinkLib                       TRUE
    linkLibType                     PRIVATE

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            systime logger configparser

    isLink3rdPartyLib               FALSE
    link3rdPartyLibsDir             
    link3rdPartyLibNeedFindPkgList  
    link3rdPartyLibNameList         
)

# 生成 device_control 可执行程序
BuildModuleExec(
    test device_control

    isAutoFindSource                FALSE
    sourceList                      device_control.cpp
    incList                         

    isLinkInc                       TRUE
    linkIncType                     PRIVATE
    linkIncList                     ${PROJECT_DIR}/utility/logger ${PROJECT_DIR}/utility/configparser ${PROJECT_DIR}/reactor

    isLinkLib                       TRUE
    linkLibType                     PRIVATE

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            logger configparser reactor

    isLink3rdPartyLib               FALSE
    link3rdPartyLibsDir             
    link3rdPartyLibNeedFindPkgList  
    link3rdPartyLibNameList         
)

# 生成 pressure_test 可执行程序
BuildModuleExec(
    test pressure_test

    isAutoFindSource                FALSE
    sourceList                      pressure_test.cpp
    incList                         

    isLinkInc                       TRUE
    linkIncType                     PRIVATE
    linkIncList                     ${PROJECT_DIR}/utility/logger ${PROJECT_DIR}/utility/configparser ${PROJECT_DIR}/reactor ${PROJECT_DIR}/utility/rabbitmqclient

    isLinkLib                       TRUE
    linkLibType                     PRIVATE

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            logger configparser event rabbitmqclient

    isLink3rdPartyLib               FALSE
    link3rdPartyLibsDir             
    link3rdPartyLibNeedFindPkgList  
    link3rdPartyLibNameList         
)

# 生成 pressure_test 可执行程序
BuildModuleExec(
    test test_googletest

    isAutoFindSource                FALSE
    sourceList                      test_googletest.cpp
    incList                         

    isLinkInc                       FALSE
    linkIncType                     PRIVATE
    linkIncList                     

    isLinkLib                       TRUE
    linkLibType                     PRIVATE

    isLinkLocalLib                  FALSE
    linkLocalLibDir                 
    linkLocalLibNameList            

    isLink3rdPartyLib               TRUE
    link3rdPartyLibsDir             /usr/lib/x86_64-linux-gnu
    link3rdPartyLibNeedFindPkgList  GTest
    link3rdPartyLibNameList         gtest.a  gtest_main.a
)