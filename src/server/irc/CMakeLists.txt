include(ExternalProject)

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  #${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
  )
  
CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Exclude
  #${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
  )

  
#custom libircclient project, still a bit dirty (read: if you can make it cleaner do it)
IF ( UNIX )
ExternalProject_Add(libircclient
    SOURCE_DIR ${CMAKE_BINARY_DIR}
    INSTALL_DIR ${SUNLIBDIR}
    CONFIGURE_COMMAND 
    #This lib is only linked by worldserver.
    #problem is: before the install step, the library isn't there (it's in build/libircclient/src) and worldserver can't link it. 
    #so actually the worldserver links it in its build folder. FIXME.
    cp -R ${CMAKE_SOURCE_DIR}/dep/libircclient ${CMAKE_BINARY_DIR}
      && chmod 700 ${CMAKE_BINARY_DIR}/libircclient/configure
      && ${CMAKE_BINARY_DIR}/libircclient/configure --prefix=${CMAKE_BINARY_DIR} --libdir=${SUNLIBDIR} --enable-openssl --enable-shared
    BINARY_DIR ${CMAKE_BINARY_DIR}/libircclient
    BUILD_COMMAND ${MAKE}) 
    #todo custom clean
endif ( UNIX )


SET(irc_srcs
    ${PRIVATE_SOURCES}
)

add_library(irc STATIC ${irc_srcs})
IF(UNIX)
	#libircclient is built on linux only (and imported on windows)
	add_dependencies(irc libircclient)
ENDIF()

target_include_directories(irc
  INTERFACE
    ${PUBLIC_INCLUDES}
	${CMAKE_SOURCE_DIR}/dep/libircclient/include/
)
	
include_directories(${CMAKE_SOURCE_DIR}/dep/libircclient/include/)

target_link_libraries(irc
  PRIVATE
    #the next one contains nothing on linux
    ${LIBIRCCLIENT_LIB_WIN}
    boost
	common
	game-interface
)

set_target_properties(irc
    PROPERTIES
      FOLDER
        "server")