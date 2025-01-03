cmake_minimum_required(VERSION 3.0...3.31)
cmake_policy(SET CMP0054 NEW) # Don't try to interpret "FOO" as a variable
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckFunctionExists)

project(ssb-dbgen
	DESCRIPTION "SSB benchmark data (and query) generator utilities"
	LANGUAGES C)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(DATABASE "DB2" CACHE STRING "Command language dialect to target with the qgen query generator (\"DATABASE\"")
set_property(CACHE DATABASE PROPERTY STRINGS INFORMIX DB2 TDAT SQLSERVER SYBASE)

set(WORKLOAD SSB CACHE STRING "Choice of benchmark / query workload (\"WORKLOAD\")")
set_property(CACHE WORKLOAD PROPERTY STRINGS TPCH SSB)

set(CSV_OUTPUT_FORMAT OFF CACHE BOOL "Use CSV output format")
set(EOL_HANDLING OFF CACHE BOOL "Skip separator after the last field in generated lines?")
set(YMD_DASH_DATE OFF CACHE BOOL "Generate date values in YYYY-MM-DD format (with dashes) rather than YYYYMMDD")

if (CSV_OUTPUT_FORMAT AND NOT EOL_HANDLING)
	message(WARNING "When CSV is used for the output format, EOL_HANDLING is ignored and a separator is _never_ printed after the last field on the line (the equivalent of EOL_HANDLING being ON).")
endif()

# Header checks...

# ... for POSIX systems
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(sys/wait.h HAVE_SYS_WAIT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(strings.h HAVE_STRINGS_H)

# ... for Windows
check_include_file(getopt.h HAVE_GETOPT_H)
check_include_file(process.h HAVE_PROCESS_H)
check_include_file(windows.h HAVE_WINDOWS_H)    

# ... non-platform-specific
check_include_file(stdint.h HAVE_STDINT_H) # standard since C99
check_include_file(sys/bittypes.h HAVE_SYS_BITTYPES_H)

check_symbol_exists(malloc stdlib.h HAVE_MALLOC_IN_STDLIB)
if (NOT HAVE_MALLOC_IN_STDLIB)
	check_include_file(malloc.h HAVE_MALLOC_H)
	if (NOT HAVE_MALLOC_H)
		message(FATAL_ERROR "Could not locate a definition of the malloc() function")
	endif()
endif()

if (NOT HAVE_UNISTD_H AND HAVE_SYS_TYPES_H)
	check_symbol_exists(pid_t sys/types.h HAVE_PID_T_IN_SYS_TYPES_H)
endif()

check_symbol_exists(getopt stdlib.h STDLIB_HAS_GETOPT)
check_symbol_exists(getenv stdlib.h STDLIB_HAS_GETENV)
if (NOT STDLIB_HAS_GETENV)
    message(FATAL_ERROR "getenv is required but not found")
endif()
#check_function_exists(getpid HAVE_GETPID)
#if (NOT HAVE_GETPID)
#	check_function_exists(_getpid HAVE__GETPID)
#endif()

# Functions necessary for parallel data generation...

# ... with POSIX

check_function_exists(kill HAVE_KILL)
check_function_exists(fork HAVE_FORK)
check_function_exists(wait HAVE_WAIT)

# ... on Windows (currently disabled, see the code)

#check_function_exists(TerminateProcess HAVE_TERMINATE_PROCESS)
#check_function_exists(spawnv HAVE_SPAWNV)
#if (NOT HAVE_SPAWNV)
#	check_function_exists(_spawnv HAVE__SPAWNV)
#endif()
#check_function_exists(cwait HAVE_CWAIT)
#if (NOT HAVE_CWAIT)
#	check_function_exists(_cwait HAVE__CWAIT)
#endif()


configure_file(src/config.h.in src/config.h @ONLY)
# The following is necessary since the generated config.h will be placed
# in the build directory ("binary" directory), not in the source directory
include_directories("${CMAKE_CURRENT_BINARY_DIR}/src")

add_executable(dbgen
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
	src/bm_utils.c
	src/build.c
	src/permute.c
	src/qgen.c
	src/rnd.c
	src/speed_seed.c
	src/text.c
	src/varsub.c
)


if (NOT LOG_FUNCTION_EXISTS AND NOT NEED_LINKING_AGAINST_LIBM)
	# Decide whether or not to link against the C math library (libm);
	# See: https://stackoverflow.com/q/32816646/1593077
	CHECK_FUNCTION_EXISTS(log LOG_FUNCTION_EXISTS)
	if(NOT LOG_FUNCTION_EXISTS)
		unset(LOG_FUNCTION_EXISTS CACHE)
		list(APPEND CMAKE_REQUIRED_LIBRARIES m)
		CHECK_FUNCTION_EXISTS(log LOG_FUNCTION_EXISTS)
		if(LOG_FUNCTION_EXISTS)
			unset(LOG_FUNCTION_EXISTS CACHE)
		    set(NEED_LINKING_AGAINST_LIBM True CACHE BOOL "" FORCE)
			message(STATUS "Need linking against the C math library")
		else()
			message(FATAL_ERROR "Cannot determine how to make the C standard library math functions available")
		endif()
	else()
	endif()
endif()

if (NEED_LINKING_AGAINST_LIBM)
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

set_property(TARGET dbgen qgen PROPERTY C_STANDARD 99)
set_property(TARGET dbgen qgen PROPERTY C_EXTENSIONS OFF)

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


