include(ExternalProject)

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  )
  
CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  )

#custom libircclient project, still a bit dirty but now seems to work seamlessly (if you can make it cleaner, do it)
IF ( UNIX )
    ExternalProject_Add(libircclient
        SOURCE_DIR ${CMAKE_BINARY_DIR}
        INSTALL_DIR ${LIBSDIR}
        CONFIGURE_COMMAND 
        #A problem with linking this is: before the install step, the library isn't there (it's in build/libircclient/src) so worldserver can't link it. 
        #Instead, worldserver links it from the output in the build folder (see cmake for project worldserver)
        cp -R ${CMAKE_SOURCE_DIR}/dep/libircclient ${CMAKE_BINARY_DIR}
          && chmod 700 ${CMAKE_BINARY_DIR}/libircclient/configure
          && ${CMAKE_BINARY_DIR}/libircclient/configure --prefix=${CMAKE_BINARY_DIR} --libdir=${LIBSDIR} --enable-openssl --enable-shared
        BINARY_DIR ${CMAKE_BINARY_DIR}/libircclient
        BUILD_COMMAND ${MAKE} 
        #todo custom clean
    )
        
    add_dependencies(irc libircclient)
endif ( UNIX )

add_library(irc STATIC
    ${PRIVATE_SOURCES}
)

target_include_directories(irc
  INTERFACE
    ${PUBLIC_INCLUDES}
    ${CMAKE_SOURCE_DIR}/dep/libircclient/include/
)
    
include_directories(${CMAKE_SOURCE_DIR}/dep/libircclient/include/)

#For Windows, use prebuilt libircclient library instead of building it
if (WIN32)
    if(PLATFORM EQUAL 64)
        set (LIBIRCCLIENT_LIB_WIN ${PROJECT_SOURCE_DIR}/dep/libircclient/prebuilt_win_lib/64/libircclient.lib)
    elseif(PLATFORM EQUAL 32)
        set (LIBIRCCLIENT_LIB_WIN ${PROJECT_SOURCE_DIR}/dep/libircclient/prebuilt_win_lib/32/libircclient.lib)
    else()
        message(SEND_ERROR "Could not find platform when trying to set LIBIRCCLIENT_LIB_WIN")
    endif()
endif()

target_link_libraries(irc
  PUBLIC
    ${LIBIRCCLIENT_LIB_WIN}
  PRIVATE
    boost
    common
    game-interface
)

set_target_properties(irc
    PROPERTIES
      FOLDER
        "server")