file(GLOB_RECURSE sources_Playerbot *.cpp *.h)

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/pch)

  
set(PRIVATE_PCH_HEADER pch/pch.h)
set(PRIVATE_PCH_SOURCE pch/pch.cpp)


GroupSources(${CMAKE_CURRENT_SOURCE_DIR})


if ( UNIX )
    add_definitions(
        -D_PLAYERBOT_CONFIG='"${CONF_DIR}/aiplayerbot.conf"'
            )
 endif()

add_library(playerbot STATIC
  ${PRIVATE_SOURCES}
  ${PRIVATE_PCH_SOURCE}
)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/pch)

target_include_directories(playerbot
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})
	
target_link_libraries(playerbot
  PRIVATE
    common
	game-interface
	)

set_target_properties(playerbot
    PROPERTIES
      FOLDER
        "server")

add_dependencies(playerbot revision_data.h)

if( WIN32 )
  if ( MSVC )
	add_custom_command(TARGET playerbot
      POST_BUILD
 	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/aiplayerbot.conf.dist.in ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    ) 
  elseif ( MINGW )
    
	 add_custom_command(TARGET playerbot
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}aiplayerbot.conf.dist.in ${CMAKE_BINARY_DIR}/bin/
    )
  endif()
endif()

if( UNIX )
  install(TARGETS playerbot DESTINATION bin)
  install(FILES aiplayerbot.conf.dist.in DESTINATION ${CONF_DIR})
elseif( WIN32 )
  install(TARGETS playerbot DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES aiplayerbot.conf.dist.in DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()

# Generate precompiled header
add_cxx_pch(playerbot ${PRIVATE_PCH_HEADER} ${PRIVATE_PCH_SOURCE})
