#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "rabbitmq::rabbitmq" for configuration "Release"
set_property(TARGET rabbitmq::rabbitmq APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(rabbitmq::rabbitmq PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/librabbitmq.4.dll.a"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/librabbitmq.4.dll"
  )

list(APPEND _cmake_import_check_targets rabbitmq::rabbitmq )
list(APPEND _cmake_import_check_files_for_rabbitmq::rabbitmq "${_IMPORT_PREFIX}/lib/librabbitmq.4.dll.a" "${_IMPORT_PREFIX}/bin/librabbitmq.4.dll" )

# Import target "rabbitmq::rabbitmq-static" for configuration "Release"
set_property(TARGET rabbitmq::rabbitmq-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(rabbitmq::rabbitmq-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/liblibrabbitmq.4.a"
  )

list(APPEND _cmake_import_check_targets rabbitmq::rabbitmq-static )
list(APPEND _cmake_import_check_files_for_rabbitmq::rabbitmq-static "${_IMPORT_PREFIX}/lib/liblibrabbitmq.4.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
