# set up output paths for executable binaries (.exe-files, and .dll-files on DLL-capable platforms)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(MSVC_EXPECTED_VERSION 19.10)
set(MSVC_EXPECTED_VERSION_STRING "MSVC 2017")

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS MSVC_EXPECTED_VERSION)
  message(FATAL_ERROR "MSVC: TrinityCore requires version ${MSVC_EXPECTED_VERSION} (${MSVC_EXPECTED_VERSION_STRING}) to build but found ${CMAKE_CXX_COMPILER_VERSION}")
endif()

# CMake sets warning flags by default, however we manage it manually
# for different core and dependency targets
string(REGEX REPLACE "/W[0-4] " "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
# Search twice, once for space after /W argument,
# once for end of line as CMake regex has no \b
string(REGEX REPLACE "/W[0-4]$" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "/W[0-4] " "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REGEX REPLACE "/W[0-4]$" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

# set up output paths ofr static libraries etc (commented out - shown here as an example only)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(PLATFORM EQUAL 64)
  # This definition is necessary to work around a bug with Intellisense described
  # here: http://tinyurl.com/2cb428.  Syntax highlighting is important for proper
  # debugger functionality.
  target_compile_definitions(trinity-compile-option-interface
    INTERFACE
	  -D_WIN64)
	  
  message(STATUS "MSVC: 64-bit platform, enforced -D_WIN64 parameter")

  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0.23026.0)
    #Enable extended object support for debug compiles on X64 (not required on X86)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
    message(STATUS "MSVC: Enabled increased number of sections in object files")
  endif()
else()
  # mark 32 bit executables large address aware so they can use > 2GB address space
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
  message(STATUS "MSVC: Enabled large address awareness")

  target_compile_options(trinity-compile-option-interface
    INTERFACE
      /arch:SSE2)
	  
  message(STATUS "MSVC: Enabled SSE2 support")

  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
  message(STATUS "MSVC: Disabled Safe Exception Handlers for debug builds")
endif()

# Set build-directive (used in core to tell which buildtype we used)
# msbuild/devenv don't set CMAKE_MAKE_PROGRAM, you can choose build type from a dropdown after generating projects
if("${CMAKE_MAKE_PROGRAM}" MATCHES "MSBuild")
  target_compile_definitions(trinity-compile-option-interface
    INTERFACE
      -D_BUILD_DIRECTIVE="$(ConfigurationName)")
else()
  # while all make-like generators do (nmake, ninja)
  target_compile_definitions(trinity-compile-option-interface
    INTERFACE
      -D_BUILD_DIRECTIVE="${CMAKE_BUILD_TYPE}")
endif()

# multithreaded compiling on VS
# Exception Handling Model: The exception-handling model that catches C++ exceptions only 
target_compile_options(trinity-compile-option-interface
  INTERFACE
    /MP
	/EHsc)
	
if((PLATFORM EQUAL 64) OR (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0.23026.0) OR BUILD_SHARED_LIBS)
  # Enable extended object support
  target_compile_options(trinity-compile-option-interface
    INTERFACE
      /bigobj)

  message(STATUS "MSVC: Enabled increased number of sections in object files")
endif()

# /Zc:throwingNew.
# When you specify Zc:throwingNew on the command line, it instructs the compiler to assume
# that the program will eventually be linked with a conforming operator new implementation,
# and can omit all of these extra null checks from your program.
# http://blogs.msdn.com/b/vcblog/archive/2015/08/06/new-in-vs-2015-zc-throwingnew.aspx
if(NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0.23026.0))
  # also enable /bigobj for ALL builds under visual studio 2015, increased number of templates in standard library 
  # makes this flag a requirement to build TC at all
  target_compile_options(trinity-compile-option-interface
    INTERFACE
      /Zc:throwingNew)
endif()

# Define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES - eliminates the warning by changing the strcpy call to strcpy_s, which prevents buffer overruns
target_compile_definitions(trinity-compile-option-interface
  INTERFACE
    -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES)
message(STATUS "MSVC: Overloaded standard names")

# Ignore warnings about older, less secure functions
target_compile_definitions(trinity-compile-option-interface
  INTERFACE
    -D_CRT_SECURE_NO_WARNINGS)
message(STATUS "MSVC: Disabled NON-SECURE warnings")

#Ignore warnings about POSIX deprecation
target_compile_definitions(trinity-compile-option-interface
  INTERFACE
    -D_CRT_NONSTDC_NO_WARNINGS)
message(STATUS "MSVC: Disabled POSIX warnings")

target_compile_options(trinity-compile-option-interface
  INTERFACE
    /std:c++14)
message(STATUS "MSVC: Enabled C++14")

# Ignore specific warnings
# C4351: new behavior: elements of array 'x' will be default initialized
# C4091: 'typedef ': ignored on left of '' when no variable is declared
# C4820: "..bytes padding added after data member.."
# C4706: assignment within conditional expression
# C4127: "conditional expression is constant" -> incorrect warning in ObjectGuid. Code is correct and will be correctly optimized by compiler. https://stackoverflow.com/questions/25573996/c4127-conditional-expression-is-constant
# C4100: unreferenced formal parameter
# C4505: unreferenced local function has been removed
# C4457: declaration of 'x' hides function parameter
target_compile_options(trinity-compile-option-interface
  INTERFACE
    /wd4351
    /wd4091
	/wd4820
	/wd4706
	/wd4127
	/wd4100
	/wd4505
	/wd4457)
	
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ")
if(DO_WARN)
	target_compile_options(trinity-warning-interface
	  INTERFACE
		/W4)
		
  message(STATUS "MSVC: Enabled level 4 warnings")
else()

  target_compile_options(trinity-warning-interface
    INTERFACE
	  /W3)
	
  # C4996?
  # C4355?
  # C4244?
  # C4985?
  # ...
  # C4838: conversion from 'type_1' to 'type_2' requires a narrowing conversion
  target_compile_options(trinity-compile-option-interface
    INTERFACE
      /wd4996
      /wd4355
      /wd4244
      /wd4985
      /wd4267
      /wd4619
      /wd4512
	  /wd4838)
endif()

if (BUILD_SHARED_LIBS)
  # C4251: needs to have dll-interface to be used by clients of class '...'
  # C4275: non dll-interface class ...' used as base for dll-interface class '...'
   target_compile_options(trinity-compile-option-interface
    INTERFACE
      /wd4251
      /wd4275)
	  
  message(STATUS "MSVC: Enabled shared linking")
endif()

# Enable and treat as errors the following warnings to easily detect virtual function signature failures:
# 'function' : member function does not override any base class virtual member function
# 'virtual_function' : no override available for virtual member function from base 'class'; function is hidden
target_compile_options(trinity-compile-option-interface
  INTERFACE
    /we4263
    /we4264)
	
# Disable incremental linking in debug builds. (sun: disabled for now, seems working)
# To prevent linking getting stuck (which might be fixed in a later VS version).
#macro(DisableIncrementalLinking variable)
  #string(REGEX REPLACE "/INCREMENTAL *" "" ${variable} "${${variable}}")
  #set(${variable} "${${variable}} /INCREMENTAL:NO")
#endmacro()

#DisableIncrementalLinking(CMAKE_EXE_LINKER_FLAGS_DEBUG)
#DisableIncrementalLinking(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO)
#DisableIncrementalLinking(CMAKE_SHARED_LINKER_FLAGS_DEBUG)
#DisableIncrementalLinking(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO)

if(DO_DEBUG)
	#Ob0 : Disable inlining
	target_compile_options(trinity-compile-option-interface
	  INTERFACE
		/Ob0)
		
endif(DO_DEBUG)
