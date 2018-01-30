cmake_minimum_required(VERSION 3.0)

project(ssb-dbgen # SSB benchmark data (and query) generator utilities
	LANGUAGES C)

set(DATABASE "DB2" CACHE STRING "Command language dialect to target with the qgen query generator (\"DATABASE\"")
set_property(CACHE DATABASE PROPERTY STRINGS INFORMIX DB2 TDAT SQLSERVER SYBASE)

set(MACHINE LINUX CACHE STRING "Platform to target (\"MACHINE\")")
set_property(CACHE MACHINE PROPERTY STRINGS ATT HP IBM ICL MVS SGI SUN U2200 VMS LINUX WIN32 MAC)

set(WORKLOAD SSB CACHE STRING "Choice of benchmark / query workload (\"WORKLOAD\")")
set_property(CACHE WORKLOAD PROPERTY STRINGS TPCH SSB)

set(EOL_HANDLING OFF CACHE BOOL "Skip separator after the last field in generated lines?")

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

target_link_libraries(dbgen m)
target_link_libraries(qgen m)

set_property(
	TARGET dbgen qgen
	APPEND PROPERTY COMPILE_DEFINITIONS
		DBNAME="dss"
		${MACHINE}
		${DATABASE}
		${WORKLOAD}
		_FILE_OFFSET_BITS=64
)

if (EOL_HANDLING)
	set_property(
		TARGET dbgen qgen
		APPEND PROPERTY COMPILE_DEFINITIONS EOL_HANDLING
	)
endif()

set_property(
	TARGET dbgen qgen
	APPEND PROPERTY COMPILE_OPTIONS
		-Wall
)
