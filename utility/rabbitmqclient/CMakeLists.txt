
BuildModuleLibrary(
    rabbitmqclient rabbitmqclient SHARED

    isAutoFindSource                TRUE
    sourceList                      
    incList                         

    isLinkInc                       TRUE
    linkIncType                     PUBLIC
    linkIncList                     /usr/include ${PROJECT_DIR}/utility/logger ${PROJECT_DIR}/utility/safecontainer

    isLinkLib                       TRUE
    linkLibType                     PUBLIC

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            logger safecontainer

    isLink3rdPartyLib               TRUE
    link3rdPartyLibsDir             /usr/lib/x86_64-linux-gnu
    link3rdPartyLibNeedFindPkgList  
    link3rdPartyLibNameList         librabbitmq.so
)