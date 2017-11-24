#To improve: Unix implementation needs to check for includes and libs existence
add_library(gperftools INTERFACE)

if(WIN32)
	MESSAGE(SEND_ERROR "gperftools is not supported on windows at the moment")
	#Build system is almost done, but can't seem to get my built lib with the server, no sure why...
	
	set(GPERFTOOLS_WIN_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/win/includes")
endif(WIN32)

target_include_directories(gperftools
  INTERFACE
	${GPERFTOOLS_WIN_INCLUDE_DIRS}
  )

if(UNIX)
  target_link_libraries(gperftools
    INTERFACE
      profiler
      tcmalloc
  )
endif(UNIX)

if (WIN32)
	if(PLATFORM EQUAL 64)
        set (TCMALLOC_LIB_WIN ${PROJECT_SOURCE_DIR}/dep/gperftools/win/prebuilt_lib/64/libtcmalloc_minimal.lib)
    elseif(PLATFORM EQUAL 32)
        set (TCMALLOC_LIB_WIN ${PROJECT_SOURCE_DIR}/dep/gperftools/win/prebuilt_lib/32/libtcmalloc_minimal.lib)
    else()
        message(SEND_ERROR "Could not find platform when trying to set TCMALLOC_LIB_WIN")
    endif()
	
	target_link_libraries(gperftools
		INTERFACE
		  ${TCMALLOC_LIB_WIN}
    )
endif()