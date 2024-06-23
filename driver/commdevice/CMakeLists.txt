
BuildModuleLibrary(
    commdevice commdevice SHARED
    
    isAutoFindSource                TRUE
    sourceList                      
    incList                         

    isLinkInc                       TRUE
    linkIncType                     PUBLIC
    linkIncList                     ${PROJECT_DIR}/utility/systime ${PROJECT_DIR}/utility/logger ${PROJECT_DIR}/utility/configparser ${PROJECT_DIR}/utility/safecontainer ${PROJECT_DIR}/driver/filetransfer ${PROJECT_DIR}/reactor

    isLinkLib                       TRUE
    linkLibType                     PUBLIC

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            logger systime configparser event filetransfer

    isLink3rdPartyLib               TRUE
    link3rdPartyLibsDir             /usr/lib/x86_64-linux-gnu
    link3rdPartyLibNeedFindPkgList  boost_system boost_filesystem
    link3rdPartyLibNameList         boost_system.so boost_filesystem.so
)