add_executable(vmap4assembler VMapAssembler.cpp)

target_link_libraries(vmap4assembler
  PRIVATE
    trinity-core-interface
  PUBLIC
    common
    zlib)

set_target_properties(vmap4assembler
    PROPERTIES
      FOLDER
        "tools")

if( UNIX )
  install(TARGETS vmap4assembler DESTINATION bin)
elseif( WIN32 )
  install(TARGETS vmap4assembler DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
