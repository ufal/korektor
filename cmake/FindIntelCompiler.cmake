#
# Try to find ICC compiler.
# Once done this will define
#
# ICC_FOUND
# ICC
# ICPC
# XIAR
# XILD
#
# and set
#
# CMAKE_C_COMPILER
# CMAKE_CXX_COMPILER
# CMAKE_C_CREATE_STATIC_LIBRARY
# CMAKE_CXX_CREATE_STATIC_LIBRARY
# CMAKE_LINKER
#

find_program(ICC icc)
if(ICC)
	set(CMAKE_C_COMPILER ${ICC})

	# c++ compiler
	find_program(ICPC icpc)
	if(ICPC)
		set(CMAKE_CXX_COMPILER ${ICPC})
	endif(ICPC)

	# archiver
	find_program(XIAR xiar)
	if(XIAR)
		set(CMAKE_C_CREATE_STATIC_LIBRARY
		"${XIAR} cr <TARGET> <LINK_FLAGS> <OBJECTS> "
		"${XIAR} -s <TARGET> ")
		set(CMAKE_CXX_CREATE_STATIC_LIBRARY
		"${XIAR} cr <TARGET> <LINK_FLAGS> <OBJECTS> "
		"${XIAR} -s <TARGET> ")
	endif()
	mark_as_advanced(XIAR)

	# linker
	find_program(XILD xild)
	if(XILD)
		set(CMAKE_LINKER "${XILD}")
	endif(XILD)
endif(ICC)

IF (ICC)
	SET( ICC_FOUND ON CACHE STRING "Set to ON if the Intel Compiler is found, OFF otherwise")
ELSE (ICC)
	SET( ICC_FOUND OFF CACHE STRING "Set to ON if the Intel Compiler is found, OFF otherwise")
ENDIF (ICC)

mark_as_advanced(ICC_FOUND)
mark_as_advanced(ICC)
mark_as_advanced(ICPC)
mark_as_advanced(XIAR)
mark_as_advanced(XILD)
