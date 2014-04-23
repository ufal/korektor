# ---------------------------------------------------------------------------------------------------
# Korektor main make file
# (C)2012 Korektor
# ---------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8)

# ---------------------------------------------------------------------------------------------------
# Include default Launcher
# ---------------------------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CreateLaunchers)


# ---------------------------------------------------------------------------------------------------
# Some default CMAKE settings
# ---------------------------------------------------------------------------------------------------
set(CMAKE_LEGACY_CYGWIN_WIN32 0) # silence warning
set(CMAKE_BUILD_TYPE Debug CACHE STRING "build as Debug, Release, or RelWithDebInfo")
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# ---------------------------------------------------------------------------------------------------
# Compiler selection and settings
# ---------------------------------------------------------------------------------------------------

#
# Default user settings for CLANG
#



if (DEFINED USE_CLANG AND USE_CLANG)
	set(USE_ICC OFF)
else (DEFINED USE_CLANG AND USE_CLANG)
	set(USE_CLANG OFF) # set this to off if you want to disable using clang
endif (DEFINED USE_CLANG AND USE_CLANG)

#
# Default user settings for INTEL compiler
#
if (DEFINED USE_ICC AND USE_ICC)
	set(USE_CLANG OFF)
else (DEFINED USE_ICC AND USE_ICC)
	set(USE_ICC OFF) # set this to off if you want to disable using ICC
endif (DEFINED USE_ICC AND USE_ICC)

#
# LLVM/CLANG compiler settings
#
if(USE_CLANG)
	find_package(CLANG)
	if (CLANG_FOUND)
		set(WITH_CLANG ON)
		set(USE_ICC OFF) # set this to off if you want to disable using ICC
	endif (CLANG_FOUND)
endif(USE_CLANG)

#
# INTEL compiler settings
#
if(USE_ICC)
	find_package(IntelCompiler)
	if(ICC_FOUND)
		set(WITH_ICC ON)
		set(USE_CLANG OFF)

		if(UNIX)
			set(ICC_RELEASE_FLAGS "-ip -ipo")
			set(ICC_RELWITHDEBINFO_FLAGS "")
		else(UNIX)
			set(ICC_RELEASE_FLAGS "/fast")
		endif(UNIX)
	endif(ICC_FOUND)
endif(USE_ICC)

# ---------------------------------------------------------------------------------------------------
# Project description (needs to happen after selecting compiler)
# ---------------------------------------------------------------------------------------------------

project(korektor)

#
# Platform specific compiler flags
#
if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif (WIN32)

if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif(UNIX)

if(USE_CLANG)
	#link_directories(${CMAKE_SOURCE_DIR}/external/libcxx/)
	# -Wformat-security warns about printf(non-const-string, ...)
	# -Woverloaded-virtual warns about virtual functions with the same name but different parameters
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-security -Wno-overloaded-virtual -Wno-reserved-user-defined-literal")
endif(USE_CLANG)

if (USE_GPROF)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif (USE_GPROF)

#
# search for Threads library
#
find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------------------------------
# Optional external libraries
# ---------------------------------------------------------------------------------------------------

#
# Check for OpenMP usage
#
if(USE_OPENMP)
	find_package(OpenMP)
	if(OPENMP_FOUND)
		set(SCRIBE_C_FLAGS "${SCRIBE_C_FLAGS} ${OpenMP_C_FLAGS}")
		set(SCRIBE_CXX_FLAGS "${SCRIBE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
		set(WITH_OPENMP ON)
	endif(OPENMP_FOUND)
endif(USE_OPENMP)

#setting ICU libraries
if (WIN32)
	set(ICU_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/icu/x86/lib/)
	set(korektor_libs ${korektor_libs} ${ICU_LIB_DIR}/icuuc.lib ${ICU_LIB_DIR}/icuio.lib)
	set(korektor_path ${korektor_path} ${CMAKE_CURRENT_SOURCE_DIR}/external/icu/x86/bin/)
else(WIN32)
	find_package(ICU 4.8 REQUIRED)
	set(korektor_incs ${korektor_incs} ${ICU_INCLUDE_DIRS})
	if (STATIC_LINKING)
		set(korektor_libs ${korektor_libs} -Wl,-Bstatic ${ICU_LIBRARIES} -Wl,-Bdynamic)
	else (STATIC_LINKING)
		set(korektor_libs ${korektor_libs} ${ICU_LIBRARIES})
	endif (STATIC_LINKING)
endif(WIN32)


set(korektor_incs ${korektor_incs}	${CMAKE_CURRENT_BINARY_DIR}
									${CMAKE_CURRENT_SOURCE_DIR}/include
									${CMAKE_CURRENT_SOURCE_DIR}/include/create_error_model)
									
include_directories(${korektor_incs})

# ---------------------------------------------------------------------------------------------------
# Define compiler flags
# ---------------------------------------------------------------------------------------------------

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${KOREKTOR_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KOREKTOR_CXX_FLAGS}")
if (WITH_THREADS)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
endif (WITH_THREADS)

# ---------------------------------------------------------------------------------------------------
# Define project source files and dependencies
# ---------------------------------------------------------------------------------------------------

#
# Korektor source files
#
file(GLOB korlib_hdrs "include/*.hpp")
file(GLOB korlib_srcs "src/*.cpp")

set(korektor_srcs ${CMAKE_CURRENT_SOURCE_DIR}/tools/korektor.cpp)
set(create_lm_binary_srcs ${CMAKE_CURRENT_SOURCE_DIR}/tools/create_lm_binary.cpp)
set(create_error_model_srcs ${CMAKE_CURRENT_SOURCE_DIR}/tools/create_error_model.cpp)
set(create_morphology_srcs ${CMAKE_CURRENT_SOURCE_DIR}/tools/create_morphology.cpp)
set(tokenize_srcs ${CMAKE_CURRENT_SOURCE_DIR}/tools/tokenize.cpp)
set(unicode_converting_srcs ${CMAKE_CURRENT_SOURCE_DIR}/tools/unicode_converting.cpp)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ICC_RELEASE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ICC_RELEASE_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${ICC_RELWITHDEBINFO_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${ICC_RELWITHDEBINFO_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

#
# UNIX specific build flags
#
if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif (UNIX)

#
# CLANG specific build flags
#
if (USE_CLANG AND USE_CXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif (USE_CLANG AND USE_CXX)

#
# Static linking
#
if (UNIX AND STATIC_LINKING)
	set(korektor_libs ${korektor_libs} -static-libstdc++ -static-libgcc)
endif (UNIX AND STATIC_LINKING)

#
# Threading
#
if (WITH_THREADS)
	set(korektor_libs ${korektor_libs} ${CMAKE_THREAD_LIBS_INIT})
endif (WITH_THREADS)


add_library(korlib STATIC ${korlib_srcs} ${korlib_hdrs})
target_link_libraries(korlib ${korektor_libs})

add_executable(korektor ${korektor_srcs})
target_link_libraries(korektor korlib ${korektor_libs})

add_executable(create_lm_binary ${create_lm_binary_srcs})
target_link_libraries(create_lm_binary korlib ${korektor_libs})

add_executable(create_error_model ${create_error_model_srcs})
target_link_libraries(create_error_model korlib ${korektor_libs})

add_executable(create_morphology ${create_morphology_srcs})
target_link_libraries(create_morphology korlib ${korektor_libs})

add_executable(tokenize ${tokenize_srcs})
target_link_libraries(tokenize korlib ${korektor_libs})

add_executable(morphology_from_h2mor ${CMAKE_CURRENT_SOURCE_DIR}/tools/morphology_from_h2mor.cpp)
target_link_libraries(morphology_from_h2mor ${korektor_libs})

if (UNIX AND WITH_REST_SERVER)
	file(GLOB korektor_server_srcs "rest_service/server/*.cpp")
	add_executable(korektor_server ${korektor_server_srcs})
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rest_service/libmicrohttpd)
	if (STATIC_LINKING)
		target_link_libraries(korektor_server korlib -pthread ${korektor_libs} -Wl,-Bstatic -licudata -Wl,-Bdynamic -ldl ${CMAKE_CURRENT_SOURCE_DIR}/rest_service/libmicrohttpd/libmicrohttpd.a)
	else (STATIC_LINKING)
		target_link_libraries(korektor_server korlib -pthread ${korektor_libs} ${CMAKE_CURRENT_SOURCE_DIR}/rest_service/libmicrohttpd/libmicrohttpd.so)
	endif (STATIC_LINKING)
endif (UNIX AND WITH_REST_SERVER)

add_executable(korektor_vertical ${CMAKE_CURRENT_SOURCE_DIR}/tools/korektor_vertical.cpp)
target_link_libraries(korektor_vertical korlib ${korektor_libs})

create_default_target_launcher(korektor RUNTIME_LIBRARY_DIRS ${korektor_path})

#add_executable(unicode_converting ${unicode_converting_srcs})
#target_link_libraries(unicode_converting korlib ${korektor_libs})
