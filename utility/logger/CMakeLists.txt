
BuildModuleLibrary(
    logger logger SHARED

    isAutoFindSource                TRUE
    sourceList                      
    incList                         

    isLinkInc                       TRUE
    linkIncType                     PUBLIC
    linkIncList                     ${PROJECT_DIR}/utility/singleton

    isLinkLib                       TRUE
    linkLibType                     PUBLIC

    isLinkLocalLib                  TRUE
    linkLocalLibDir                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    linkLocalLibNameList            

    isLink3rdPartyLib               TRUE
    link3rdPartyLibsDir             /usr/lib/x86_64-linux-gnu
    link3rdPartyLibNeedFindPkgList  spdlog fmt
    link3rdPartyLibNameList         spdlog.so fmt.so
)