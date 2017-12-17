option(DO_DEBUG "Debug mode (No optimization and debug symbols)" 0)
option(DO_WARN "Enable all compilation warnings" 0)
option(TOOLS "Build map/vmap/mmap extraction/assembler tools" 0)
option(PLAYERBOT "Include playerbot system" 1)
option(TESTS "Include tests fonctionalities" 1)
if(TESTS AND NOT PLAYERBOT)
	message("Tests are enabled, playerbot system is needed and will be compiled too")
	set(PLAYERBOT ON CACHE BOOL "Include playerbot system" FORCE)
endif(TESTS AND NOT PLAYERBOT)

option(WITH_DYNAMIC_LINKING "Enable dynamic library linking." 0)
IsDynamicLinkingRequired(WITH_DYNAMIC_LINKING_FORCED)
if (WITH_DYNAMIC_LINKING AND WITH_DYNAMIC_LINKING_FORCED)
  set(WITH_DYNAMIC_LINKING_FORCED OFF)
endif()
if (WITH_DYNAMIC_LINKING OR WITH_DYNAMIC_LINKING_FORCED)
  set(BUILD_SHARED_LIBS ON)
else()
  set(BUILD_SHARED_LIBS OFF)
endif()

if(UNIX)
	#not working on windows atm
	option(USE_GPERFTOOLS "Include profiling capabilities from gperftools" 0)
endif()
option(LICH_KING "NYI Lich King realm" 0)
#more clang options 
if(DO_DEBUG AND CLANG_COMPILER)
option(CLANG_ADDRESS_SANITIZER "Enable clang AddressSanitizer (~2x slowdown)" 0)
option(CLANG_THREAD_SANITIZER "Enable clang ThreadSanitizer (~5-15x slowdown and 5-10x memory overhead)" 0)
option(CLANG_MEMORY_SANITIZER "Enable clang MemorySanitizer (~3x slowdown)" 0)
option(CLANG_THREAD_SAFETY_ANALYSIS "Enable clang Thread Safety Analysis (compile time only)" 0)
endif()


set(SCRIPTS "static" CACHE STRING "Build core with scripts (recommanded static for production environnement")
set(SCRIPTS_AVAILABLE_OPTIONS none static dynamic minimal-static minimal-dynamic)
set_property(CACHE SCRIPTS PROPERTY STRINGS ${SCRIPTS_AVAILABLE_OPTIONS})

# Build a list of all script modules when -DSCRIPT="custom" is selected
GetScriptModuleList(SCRIPT_MODULE_LIST)
foreach(SCRIPT_MODULE ${SCRIPT_MODULE_LIST})
  ScriptModuleNameToVariable(${SCRIPT_MODULE} SCRIPT_MODULE_VARIABLE)
  set(${SCRIPT_MODULE_VARIABLE} "default" CACHE STRING "Build type of the ${SCRIPT_MODULE} module.")
  set_property(CACHE ${SCRIPT_MODULE_VARIABLE} PROPERTY STRINGS default disabled static dynamic)
endforeach()

set(WITH_SOURCE_TREE    "hierarchical" CACHE STRING "Build the source tree for IDE's.")
set_property(CACHE WITH_SOURCE_TREE PROPERTY STRINGS no flat hierarchical hierarchical-folders)