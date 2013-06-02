IF ( UNIX )
SET(malloc_STAT_SRCS
	jemalloc_linux.c
	rb.h
)
add_definitions(-O3 -DPIC -fPIC -DMALLOC_PRODUCTION -D__isthreaded=true)
add_library(malloc STATIC ${malloc_STAT_SRCS})
target_link_libraries(
	malloc
)
ENDIF ()