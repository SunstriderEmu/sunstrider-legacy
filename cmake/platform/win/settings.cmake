add_definitions(-D_WIN32_WINNT=0x0601)

# Package overloads
set(BZIP2_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/dep/bzip2")
set(ZLIB_INCLUDE_DIR " ${PROJECT_SOURCE_DIR}/dep/zlib")
set(BZIP2_LIBRARIES "bzip2")
set(ZLIB_LIBRARIES "zlib")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/msvc/settings.cmake)
#elseif (CMAKE_CXX_PLATFORM_ID MATCHES "MinGW")
  #include(${CMAKE_SOURCE_DIR}/cmake/compiler/mingw/settings.cmake)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/clang/settings.cmake)
endif()
