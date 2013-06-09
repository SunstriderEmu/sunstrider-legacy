include(ExternalProject)
IF ( UNIX )
ExternalProject_Add(libircclient
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/dep/libircclient
    CONFIGURE_COMMAND ${CMAKE_SOURCE_DIR}/dep/libircclient/configure --prefix=<INSTALL_DIR> --enable-openssl --enable-shared
    BINARY_DIR ${CMAKE_SOURCE_DIR}/dep/libircclient
    BUILD_COMMAND ${MAKE})
endif ( UNIX )

SET(wrchat_srcs
    IRCMgr.cpp
)

add_library(wrchat STATIC ${wrchat_srcs})
target_link_libraries(
    wrchat
)
