
cmake_minimum_required ( VERSION 3.0 )
project ( DFTD )

include ( FindPkgConfig )

# standard project definitions and versions
set ( DFTD_VERSION_MAJOR 0 )
set ( DFTD_VERSION_MINOR 4 )
set ( DFTD_VERSION_PATCH 0 )
set ( DFTD_VERSION_RELEASE_TYPE "dev" )

# minimum versions, requirements, policies
if ( NOT CMAKE_VERSION VERSION_LESS 3.0 )
    cmake_policy ( SET CMP0025 NEW )
endif ()
if ( NOT CMAKE_VERSION VERSION_LESS 3.2.2 )
    cmake_policy ( SET CMP0042 OLD )
    cmake_policy ( SET CMP0046 OLD)
endif ()
set ( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )


# status, directories
if ( DFTD_VERBOSE )
    message ( STATUS "Project source dir = ${PROJECT_SOURCE_DIR}" )
endif ()
message ( STATUS "Project build dir = ${CMAKE_BINARY_DIR}" )

if ( "${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}" )
    message ( FATAL_ERROR "Not allowed to run in-source builds!" )
endif ()


# build types: Release | Debug | RelWithDebInfo | MinSizeRel | Profile
if ( NOT CMAKE_BUILD_TYPE )
    set ( CMAKE_BUILD_TYPE "Release" )
endif ()

if ( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set ( DEBUGMODE ON )
    add_definitions ( "-DDEBUG" )
    if ( UNIX AND CMAKE_COMPILER_IS_GNUCC )
        add_definitions ( "-O0 -g2 -ggdb" )
    endif ()
elseif ( CMAKE_BUILD_TYPE STREQUAL "RelWidhDebInfo" )
    add_definitions ( "-DNDEBUG" )
elseif ( CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" AND CMAKE_COMPILER_IS_GNUCC )
    add_definitions ( "-Os -DNDEBUG" )
elseif ( CMAKE_BUILD_TYPE STREQUAL "Profile" AND CMAKE_COMPILER_IS_GNUCC )
    add_definitions ( "-g -pg -O3" )
endif ()


# organize targets into folders
option ( CMAKE_USE_FOLDERS "Use the FOLDER target property to organize targets into folders." ON )
mark_as_advanced ( CMAKE_USE_FOLDERS )
if ( CMAKE_USE_FOLDERS )
    set_property ( GLOBAL PROPERTY USE_FOLDERS ON )
endif ()



# option details, output control, C++ mode
set ( DFTD_VERBOSE ON CACHE BOOL "Extra compiler messages when building." )
set ( DFTD_BUILD_LIBCPLUSPLUS OFF CACHE BOOL "Compile with clang libc++." )
set ( DFTD_BUILD_TOOLS ON CACHE BOOL "Build dftd extra developer tools." )
set ( DFTD_BUILD_TESTS ON CACHE BOOL "Build dftd code tests." )

# turn on more detailed warnings and consider warnings as errors
set ( DFTD_STOP_ON_WARNING OFF CACHE BOOL "Stop building IF there are any compiler warnings" )
if ( NOT MSVC )
    add_definitions ( "-Wall" )
    if ( DFTD_STOP_ON_WARNING )
        add_definitions ( "-Werror" )
    endif()
endif ()

set ( VERBOSE ${DFTD_VERBOSE} )


# figure out which compiler we're using
if ( CMAKE_COMPILER_IS_GNUCC )
    execute_process ( COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
                      OUTPUT_VARIABLE GCC_VERSION
                      OUTPUT_STRIP_TRAILING_WHITESPACE )
    if ( DFTD_VERBOSE )
        message ( STATUS "Using gcc ${GCC_VERSION} as the compiler" )
    endif ()
endif ()


# tricky compiler cases, clang, clang (Apple), Intel ICC
if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER MATCHES "[Cc]lang" )
    # If using any flavour of clang, SET CMAKE_COMPILER_IS_CLANG and deal 
    # with it. If it's Apple clang, SET CMAKE_COMPILER_IS_APPLECLANG 
    set ( CMAKE_COMPILER_IS_CLANG 1 )
    execute_process ( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
    if ( clang_full_version_string MATCHES "Apple" )
        set ( CMAKE_CXX_COMPILER_ID "AppleClang" )
        set ( CMAKE_COMPILER_IS_APPLECLANG 1 )
        string ( REGEX REPLAC ".* version ([0-9]+\\.[0-9]+).*" "\\1" APPLECLANG_VERSION_STRING ${clang_full_version_string} )
        if ( DFTD_VERBOSE )
            message ( STATUS "The compiler is Clang: ${CMAKE_CXX_COMPILER_ID} version ${APPLECLANG_VERSION_STRING}" )
        endif ()
    else ()
        string ( REGEX REPLACE ".* version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string} )
        if ( DFTD_VERBOSE )
            message ( STATUS "The compiler is Clang: ${CMAKE_CXX_COMPILER_ID} version ${CLANG_VERSION_STRING}" )
        endif ()
    endif ()
elseif ( CMAKE_CXX_COMPILER_ID MATCHES "Intel" )
    set ( CMAKE_COMPILER_IS_INTEL 1 )
    if ( DFTD_VERBOSE )
        message ( STATUS "Using Intel ICC as the compiler" )
    endif ()
endif ()


# Options common to gcc and clang
if ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG )
    # Cmake doesn't know what to do with include_directories(SYSTEM) in
    # these cases
    set ( CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem " )
    # Ensure this macro is SET for stdint.h
    add_definitions ( "-D__STDC_LIMIT_MACROS" )
    # this allows native instructions to be used for sqrtf instead of a 
    # function call
    add_definitions ( "-fno-math-errno" )
    if ( HIDE_SYMBOLS AND NOT DEBUGMODE )
        # turn default symbol visibility to hidden
        set ( VISIBILITY_COMMAND "-fbisibility=hidden -fbisibility-inlines-hidden" )
    endif ()
endif ()


# Clang-specific options
if ( CMAKE_COMPILER_IS_CLANG OR CMAKE_COMPILER_IS_APPLECLANG )
    # disable some warnings for clang
    add_definitions ( "-Wno-unused-function" )
    add_definitions ( "-Wno-overloaded-virtual" )
    add_definitions ( "-Wno-unneeded-internal-declaration" )
    add_definitions ( "-Wno-tautological-compare" )
    add_definitions ( "-Wno-unused-private-field" )
    # disable warning about unused command line arguments
    add_definitions ( "-Qunused-arguments" )
    # don't warn IF we ask it not to warn about warnings it doesn't know
    add_definitions ( "-Wunknown-warning-option" )
    add_definitions ( "-Wno-unknown-pragmas" )
    if ( DEBUGMODE )
        add_definitions ( "-Wno-unused-function -Wno-unused-variable" )
    endif ()
    if ( CLANG_VERSION_STRINGVERSION_GREATER 3.5 OR
        APPLECLANG_VERSION_STRING VERSION_GREATER 6.1 )
        add_definitions ( "-Wno-unused-local-typedefs" )
    endif ()
endif ()


# GCC specific options
if ( CMAKE_COMPILER_IS_GNUCC AND ( NOT CMAKE_COMPILER_IS_CLANG ) )
    if ( NOT ${GCC_VERSION} VERSION_LESS 4.8 )
         add_definitions ( "-Wno-error=strict-overflow" )
    endif ()
    if ( NOT ${GCC_VERSION} VERSION_LESS 4.5 )
         add_definitions ( "-Wno-unused-result" )
    endif ()
endif ()


# windows 32/64 specific stuff
if (WIN32)
	# We don't use FindGLEW on windows, as it doesn't work.
	set ( GLEW_LIBRARY "${PROJECT_SOURCE_DIR}/build/win64/glew/lib/x64/glew32.lib" )
	set ( GLEW_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/build/win64/glew/include" )
	include_directories ( ${GLEW_INCLUDE_DIR} )
    set ( LIBS ${LIBS} ${GLEW_LIBRARY} )
	add_definitions ( "-DNOMINMAX" )
	message ( INFO " : CMake uses windows platform : " ${CMAKE_VS_PLATFORM_NAME} )
	set (CMAKE_EXE_LINKER_FLAGS "/machine:x64")	# fixme try
	if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		message ( INFO "Building Win64 (x64) version." )
	else ()
		message ( INFO "Building Win32 (x86) version." )
	endif ()
endif ()


# try to detect IF this is an osx distribution new enough that the system
# library is libc++
if ( EXISTS "/usr/lib/libc++.dylib" )
     set (DFTD_SYSTEM_HAS_LIBCPP ON )
endif ()


# use ccache IF found
find_program ( CCACHE_FOUND ccache )
if ( CCACHE_FOUND )
    set_property ( GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache )
    set_property ( GLOBAL PROPERTY RULE_LAUNCH_LINK ccache )
endif ()      


# Test Endianess of System
include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if (IS_BIG_ENDIAN)
	add_definitions ( "-DBIG_ENDIAN" )
else ()
	add_definitions ( "-DLITTLE_ENDIAN" )
endif ()


set ( EXTRA_CPP_ARGS "" CACHE STRING "Extra C++ command line definitions." )


set ( USE_FLOAT_FFTW3 ON CACHE BOOL "Use single or double precision FFTW3, pending availability" )
set ( USE_LIB64_SUFFIX ON CACHE BOOL "Use lib64 instead of lib on x86_64 systems" )
set ( USE_EFENCE OFF CACHE BOOL "Use ElectricFence if available." )


# set property for 64bits unices, debian does not follow the convention and
# puts 32bits libraries in lib32/ whilst symlinking lib64/ to lib/
if ( UNIX )
    if ( USE_LIB64_SUFFIX )
        set ( FIND_LIBRARY_USE_LIB64_PATHS ON )
    else ()
        set ( FIND_LIBRARY_USE_LIB64_PATHS OFF )
    endif ()
endif ()


# Add extra cmake modules
set (CMAKE_MODULE_PATH
     "${PROJECT_SOURCE_DIR}/src/cmake/modules"
     "${PROJECT_SOURCE_DIR}/src/cmake")


# extra arguments
if ( EXTRA_CPP_ARGS )
    message ( STATUS "Extra C++ args: ${EXTRA_CPP_ARGS}" )
    add_definitions ( "${EXTRA_CPP_ARGS}" )
endif ()

# Use C++17
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
# hack for old ubuntu
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")


# check libc++
if ( USE_LIBCPLUSPLUS AND CMAKE_COMPILER_IS_CLANG )
    message ( STATUS "Using libc++" )
    add_definitions ( "-stdlib=libc++" )
endif ()


include ( platform )
include ( externalpackages )



# paths for installation customization
# some checks might be needed for other platforms
#
set ( DEFAULT_INSTALL_PREFIX     "/usr/local" )
set ( DEFAULT_BIN_INSTALL_DIR    "bin" )                  
set ( DEFAULT_DATA_INSTALL_DIR   "share/dangerdeep" )

# build version from branch and hash
# this might require some work for osx, and specially windows
#
execute_process ( COMMAND git rev-parse --abbrev-ref HEAD
                          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                          OUTPUT_VARIABLE GIT_BRANCH
                          OUTPUT_STRIP_TRAILING_WHITESPACE
                          )

execute_process ( COMMAND git log -1 --format=%h
                          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                          OUTPUT_VARIABLE GIT_COMMIT_HASH
                          OUTPUT_STRIP_TRAILING_WHITESPACE
                          )

# add data location and version
string ( CONCAT DFTD_VERSION ${GIT_BRANCH} "-" ${GIT_COMMIT_HASH} )

string ( CONCAT DFTD_DATADIR ${DEFAULT_INSTALL_PREFIX} "/"
                             ${DEFAULT_DATA_INSTALL_DIR} "/" )
#if (WIN32)
	string ( CONCAT DFTD_DATADIR ${PROJECT_SOURCE_DIR} "/data/" )
#endif ()

# we define it twice because old build system uses the definitions without the DFTD_ prefix
add_definitions ( "-DDFTD_VERSION=\"${DFTD_VERSION}\"" "-DVERSION=\"${DFTD_VERSION}\"" )
add_definitions ( "-DDFTD_DATADIR=\"${DFTD_DATADIR}\"" "-DDATADIR=\"${DFTD_DATADIR}\"" )


# install tree
#
if ( UNIX AND NOT SELF_CONTAINED_INSTALL_TREE )
    # assume standard filesystem directory hierarchy
    set ( DEFAULT_DOC_INSTALL_DIR "share/doc/dangerdeep" )
    set ( DEFAULT_MAN_INSTALL_DIR "share/man/dangerdeep" )
endif ()

if ( EXEC_INSTALL_PREFIX )
    set ( DEFAULT_BIN_INSTALL_DIR "${EXEC_INSTALL_PREFIX}/${DEFAULT_BIN_INSTALL_DIR}" )
endif ()



# Tell cmake to process the sub-directories
add_subdirectory ( src )

#if ( DFTD_BUILD_TOOLS )
#    add_subdirectory ( src/tools )
#endif ()


# Packaging
set ( CPACK_PACKAGE_VERSION_MAJOR ${DFTD_VERSION_MAJOR} )
set ( CPACK_PACKAGE_VERSION_MINOR ${DFTD_VERSION_MINOR} )
set ( CPACK_PACKAGE_VERSION_PATCH ${DFTD_VERSION_PATCH} )
set ( CPACK_PACKAGE_VERSION_RELEASE_TYPE ${DFTD_VERSION_RELEASE_TYPE} )
set ( CPACK_PACKAGE_VENDOR "Atlantic Cods Inc" )
# License
file ( COPY "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${CMAKE_BINARY_DIR}" )
file ( RENAME "${CMAKE_BINARY_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/License.txt" )
set ( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/License.txt" )

file ( COPY "${PROJECT_SOURCE_DIR}/README" DESTINATION "${CMAKE_BINARY_DIR}" )
file ( RENAME "${CMAKE_BINARY_DIR}/README" "${CMAKE_BINARY_DIR}/Readme.txt" )
set ( CPACK_RESOURCE_FILE_README "${CMAKE_BINARY_DIR}/Readme.txt" )

# common package options
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    set ( CPACK_GENERATOR "TGZ;STGZ;RPM;DEB" )
    set ( CPACK_SOURCE_GENERATOR "TGZ" )
endif ()

if ( APPLE )
    set ( CPACK_GENERATOR "TGZ;STGZ;PackageMaker" )
    set ( CPACK_SOURCE_GENERATOR "TGZ" )
endif ()

# for packaing use major.minor.patch versioning
#
set ( CPACK_SOURCE_PACKAGE_FILE_NAME DFTD-${DFTD_VERSION_MAJOR}.${DFTD_VERSION_MINOR}.${DFTD_VERSION_PATCH}-source )

set ( CPACK_SOURCE_IGNORE_FILES ".*~*" )
include ( CPack )
