# - Find MySQL
# Find the MySQL includes and client LIBRARIES
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
# Lot of adustmens by Michal Cihar <michal@cihar.com>
#
# vim: expandtab sw=4 ts=4 sts=4:
#
# Redistribution and use is allowed according to the terms of the BSD license.

MACRO(FIND_MYSQL)
if(UNIX) 
    set(MYSQL_CONFIG_PREFER_PATH "$ENV{MYSQL_HOME}/bin" CACHE FILEPATH
        "preferred path to MySQL (mysql_config)")
    find_program(MYSQL_CONFIG mysql_config
        ${MYSQL_CONFIG_PREFER_PATH}
        /usr/local/mysql/bin/
        /usr/local/bin/
        /usr/bin/
        )
    
    if(MYSQL_CONFIG) 
        message(STATUS "Using mysql-config: ${MYSQL_CONFIG}")
        # set INCLUDE_DIR
        exec_program(${MYSQL_CONFIG}
            ARGS --include
            OUTPUT_VARIABLE MY_TMP)

        string(REGEX REPLACE "-I([^ ]*)( .*)?" "\\1" MY_TMP "${MY_TMP}")

        set(MYSQL_ADD_INCLUDE_DIR ${MY_TMP} CACHE FILEPATH INTERNAL)

        # set LIBRARIES_DIR
        exec_program(${MYSQL_CONFIG}
            ARGS --libs_r
            OUTPUT_VARIABLE MY_TMP)

        set(MYSQL_ADD_LIBRARIES "")

        string(REGEX MATCHALL "-l[^ ]*" MYSQL_LIB_LIST "${MY_TMP}")
        foreach(LIB ${MYSQL_LIB_LIST})
            string(REGEX REPLACE "[ ]*-l([^ ]*)" "\\1" LIB "${LIB}")
            list(APPEND MYSQL_ADD_LIBRARIES "${LIB}")
        endforeach(LIB ${MYSQL_LIBS})

        set(MYSQL_ADD_LIBRARIES_PATH "")

        string(REGEX MATCHALL "-L[^ ]*" MYSQL_LIBDIR_LIST "${MY_TMP}")
        foreach(LIB ${MYSQL_LIBDIR_LIST})
            string(REGEX REPLACE "[ ]*-L([^ ]*)" "\\1" LIB "${LIB}")
            list(APPEND MYSQL_ADD_LIBRARIES_PATH "${LIB}")
        endforeach(LIB ${MYSQL_LIBS})

    else(MYSQL_CONFIG)
        set(MYSQL_ADD_LIBRARIES "")
        list(APPEND MYSQL_ADD_LIBRARIES "mysqlclient_r")
    endif(MYSQL_CONFIG)
else(UNIX)
    set(MYSQL_ADD_INCLUDE_DIR "c:/msys/local/include" CACHE FILEPATH INTERNAL)
    set(MYSQL_ADD_LIBRARIES_PATH "c:/msys/local/lib" CACHE FILEPATH INTERNAL)
ENDIF(UNIX)

find_path(MYSQL_INCLUDE_DIR
  NAMES
    mysql.h
  PATHS
    ${MYSQL_ADD_INCLUDE_PATH}
    /usr/include
    /usr/include/mysql
    /usr/local/include
    /usr/local/include/mysql
    /usr/local/mysql/include
  DOC
    "Specify the directory containing mysql.h."
)

foreach(LIB ${MYSQL_ADD_LIBRARIES})
    find_library( MYSQL_LIBRARIES
      NAMES
        mysql libmysql ${LIB}
      PATHS
        ${MYSQL_ADD_LIBRARIES_PATH}
        /usr/lib
        /usr/lib/mysql
        /usr/local/lib
        /usr/local/lib/mysql
        /usr/local/mysql/lib
      DOC "Specify the location of the mysql LIBRARIES here."
    )
  endforeach(LIB ${MYSQL_ADD_LIBRARIES})

find_library( MYSQL_EXTRA_LIBRARIES
    NAMES
      z zlib
    PATHS
      /usr/lib
      /usr/local/lib
    DOC
      "if more libraries are necessary to link in a MySQL client (typically zlib), specify them here."
  )

if( MYSQL_LIBRARIES )
  if( MYSQL_INCLUDE_DIR )
    set( MYSQL_FOUND 1 )
    message(STATUS "Found MySQL LIBRARIES ${MYSQL_LIBRARIES}")
    message(STATUS "Found MySQL headers: ${MYSQL_INCLUDE_DIR}")
  else( MYSQL_INCLUDE_DIR )
    message(FATAL_ERROR "Could not find MySQL headers! Please install the development libraries and headers")
  endif( MYSQL_INCLUDE_DIR )
  mark_as_advanced( MYSQL_FOUND MYSQL_LIBRARIES MYSQL_EXTRA_LIBRARIES MYSQL_INCLUDE_DIR )
else( MYSQL_LIBRARIES )
  message(FATAL_ERROR "Could not find the MySQL libraries! Please install the development libraries and headers")
endif( MYSQL_LIBRARIES )
ENDMACRO(FIND_MYSQL)
