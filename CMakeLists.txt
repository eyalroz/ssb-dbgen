cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0054 NEW) # Don't try to interpret "FOO" as a variable
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckFunctionExists)


project(ssb-dbgen # SSB benchmark data (and query) generator utilities
	LANGUAGES C)


if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(DATABASE "DB2" CACHE STRING "Command language dialect to target with the qgen query generator (\"DATABASE\"")
set_property(CACHE DATABASE PROPERTY STRINGS INFORMIX DB2 TDAT SQLSERVER SYBASE)

set(WORKLOAD SSB CACHE STRING "Choice of benchmark / query workload (\"WORKLOAD\")")
set_property(CACHE WORKLOAD PROPERTY STRINGS TPCH SSB)

set(EOL_HANDLING OFF CACHE BOOL "Skip separator after the last field in generated lines?")

check_type_size("long long"        SIZEOF_LONG_LONG)
check_type_size("long"             SIZEOF_LONG)
check_type_size("int"              SIZEOF_INT)
check_type_size("short"            SIZEOF_SHORT)

check_include_file(strings.h       HAVE_STRINGS_H)
check_include_file(inttypes.h      HAVE_INTTYPES_H)
check_include_file(stdint.h        HAVE_STDINT_H)
check_include_file(malloc.h        HAVE_MALLOC_H)
check_include_file(sys/bittypes.h  HAVE_SYS_BITTYPES_H)

check_function_exists(getopt       HAVE_GETOPT)
check_function_exists(getpid       HAVE_GETPID)

configure_file(src/config.h.in src/config.h @ONLY)
# The following is necessary since the generated config.h will be placed
# in the build directory ("binary" directory), not in the source directory
include_directories("${CMAKE_CURRENT_BINARY_DIR}/src")

add_executable(dbgen
	src/bcd2.c
	src/bm_utils.c
	src/build.c
	src/driver.c
	src/load_stub.c
	src/permute.c
	src/print.c
	src/rnd.c
	src/speed_seed.c
	src/text.c
)

add_executable(qgen
	src/bcd2.c
	src/bm_utils.c
	src/build.c
	src/permute.c
	src/qgen.c
	src/rnd.c
	src/speed_seed.c
	src/text.c
	src/varsub.c
)

# Link against the C math library (libm)
find_library(HAVE_LIBM m)
if (HAVE_LIBM)
	target_link_libraries(dbgen m)
	target_link_libraries(qgen m)
endif()

set_property(
	TARGET dbgen qgen
	APPEND PROPERTY COMPILE_DEFINITIONS
		DBNAME="dss"
		${DATABASE}
		${WORKLOAD}
		_FILE_OFFSET_BITS=64
)

if (EOL_HANDLING)
	set_property( TARGET dbgen qgen APPEND PROPERTY COMPILE_DEFINITIONS EOL_HANDLING)
endif()

set_property(TARGET dbgen qgen APPEND PROPERTY C_STANDARD 99)
set_property(TARGET dbgen qgen APPEND PROPERTY C_EXTENSIONS OFF)

# Note: Defining _POSIX_C_SOURCE=200809L allows the code access to functions defined in the POSIX standard,
# but not in the C language standard - and such functions _are_ in use in the code.
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set_property(TARGET dbgen qgen APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wno-missing-field-initializers)
    set_property(TARGET dbgen qgen APPEND PROPERTY COMPILE_DEFINITIONS _POSIX_C_SOURCE=200809L)
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set_property(TARGET dbgen qgen APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wno-missing-field-initializers)
    set_property(TARGET dbgen qgen APPEND PROPERTY COMPILE_DEFINITIONS _POSIX_C_SOURCE=200809L)
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    set_property(TARGET dbgen qgen APPEND PROPERTY COMPILE_OPTIONS "/W3")
    set_property(TARGET dbgen qgen APPEND PROPERTY COMPILE_DEFINITIONS _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS)
endif()

