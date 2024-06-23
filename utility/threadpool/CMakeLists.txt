
BuildModuleLibrary(
    threadpool threadpool SHARED
    
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

    isLink3rdPartyLib               FALSE
    link3rdPartyLibsDir             
    link3rdPartyLibNeedFindPkgList  
    link3rdPartyLibNameList         
)