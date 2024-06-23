
BuildModuleLibrary(
    configparser configparser SHARED

    isAutoFindSource                TRUE
    sourceList                      
    incList                         

    isLinkInc                       TRUE
    linkIncType                     PUBLIC
    linkIncList                     ${PROJECT_DIR}/utility/logger

    isLinkLib                       TRUE
    linkLibType                     PUBLIC

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            logger

    isLink3rdPartyLib               TRUE
    link3rdPartyLibsDir             /usr/lib/x86_64-linux-gnu
    link3rdPartyLibNeedFindPkgList  boost_filesystem
    link3rdPartyLibNameList         boost_filesystem.so
)

target_compile_definitions(configparser
PRIVATE
    BOOST_BIND_GLOBAL_PLACEHOLDERS
)