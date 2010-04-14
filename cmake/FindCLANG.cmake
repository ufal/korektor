#
# Try to find CLANG compiler.
# Once done this will define
#
# CLANG_FOUND
# CLANG
# CLANGPP
#
# and set CMAKE_C_COMPILER and CMAKE_CXX_COMPILER
#

find_program(CLANG clang)
find_program(CLANGXX clang++)
if (CLANG AND CLANGXX)
	set(CMAKE_C_COMPILER ${CLANG})

	# c++ compiler
	set(CMAKE_CXX_COMPILER ${CLANGXX})

#	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Winternal-linkage-in-inline")
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winternal-linkage-in-inline")
endif(CLANG AND CLANGXX)

IF (CLANG)
	SET( CLANG_FOUND ON CACHE STRING "Set to ON if CLANG is found, OFF otherwise")
ELSE (CLANG)
	SET( CLANG_FOUND OFF CACHE STRING "Set to ON if CLANG is found, OFF otherwise")
ENDIF (CLANG)

MARK_AS_ADVANCED( CLANG_FOUND )
MARK_AS_ADVANCED( CLANG CLANGXX )
