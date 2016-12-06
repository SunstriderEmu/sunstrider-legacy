# Set build-directive (used in core to tell which buildtype we used)
add_definitions(-D_BUILD_DIRECTIVE='"$(CONFIGURATION)"')

if(DO_WARN)
  set(WARNING_FLAGS "-W -Wall -Wextra -Winit-self -Wfatal-errors -Wno-mismatched-tags")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -Woverloaded-virtual")
  message(STATUS "Clang: All warnings enabled")
endif()

if(DO_DEBUG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0")
  message(STATUS "Clang: Debug-flags set (-g3)")

  #http://clang.llvm.org/docs/AddressSanitizer.html
  if(CLANG_ADDRESS_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address -fno-sanitize-recover=undefined,integer -fsanitize=return -fsanitize=bounds -fsanitize=shift -fsanitize=bool -fsanitize=enum")	
    message(STATUS "/!\\ Clang: AddressSanitizer enabled. Except SLOWDOWNS on runtime.")
  endif() 
  if(CLANG_THREAD_SANITIZER)
    #Q: When I run the program under gdb, it says: FATAL: ThreadSanitizer can not mmap the shadow memory (something is mapped at 0x555555554000 < 0x7cf000000000). What to do? Run as:
    # gdb -ex 'set disable-randomization off'

    if(USE_GPERFTOOLS)
        MESSAGE(SEND_ERROR CLANG_THREAD_SANITIZER cannot be enabled with gperftools, it would conflict with tcmalloc)
    endif()     
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fPIE")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
    message(STATUS "/!\\ Clang: ThreadSanitizer enabled. Expect LARGE SLOWDOWNS on runtime.")
  endif()
  if(CLANG_MEMORY_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fno-omit-frame-pointer")	
    message(STATUS "/!\\ Clang: MemorySanitizer enabled. Except SLOWDOWNS on runtime.")
  endif()
  if(CLANG_THREAD_SAFETY_ANALYSIS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wthread-safety")	
    message(STATUS "Clang: Thread safety analysis enabled")
  endif()
endif()

# -Wno-narrowing needed to suppress a warning in g3d
# -Wno-deprecated-register is needed to suppress 185 gsoap warnings on Unix systems.
# -Wno-switch because I find this warning useless
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-narrowing -Wno-deprecated-register -Wno-switch")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1")

if (BUILD_SHARED_LIBS)
  # -fPIC is needed to allow static linking in shared libs.
  # -fvisibility=hidden sets the default visibility to hidden to prevent exporting of all symbols.
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fvisibility=hidden")

  # --no-undefined to throw errors when there are undefined symbols
  # (caused through missing TRINITY_*_API macros).
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --no-undefined")

  message(STATUS "Clang: Disallow undefined symbols")
endif()
