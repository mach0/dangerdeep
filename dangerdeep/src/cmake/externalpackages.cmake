
# Be quiet!!
if ( NOT DFTD_VERBOSE )
    set (FFTW3_FIND_QUIETLY TRUE)
    set (OpenGL_FIND_QUIETLY TRUE)
    set (OpenMP_FIND_QUIETLY TRUE)
    set (PkgConfig_FIND_QUIETLY TRUE)
    set (TinyXML_FIND_QUIETLY TRUE)
    set (ZLIB_FIND_QUIETLY TRUE)
endif ()


# custom build flags
#
#SETUP_STRING ( SPECIAL_COMPILE_FLAGS "" "Custom compilation flags" )
#if ( SPECIAL_COMPILE_FLAGS )
#    add_definitions ( ${SPECIAL_COMPILE_FLAGS} )
#endif ()


# do test for float fftw3 later
#
find_package ( FFTW3 REQUIRED single )
if ( FFTW3_FOUND )
    message ( STATUS "FFTW3 found." )
    include_directories ( ${FFTW3_INCLUDE_DIR} )
    set ( LIBS ${LIBS} ${FFTW3_LIBRARY} ${FFTW3F_LIBRARY} )	# the adding of fft3f_lib does the trick!
else ()
    message ( FATAL_ERROR "FFTW3 not found! This will fail." )
endif ()


# main OpenGL test
#
find_package ( OpenGL REQUIRED )
if ( OPENGL_FOUND )
    message ( STATUS "OpenGL found." )
    include_directories ( ${OPENGL_INCLUDE_DIR} )
    set ( LIBS ${LIBS} ${OPENGL_LIBRARY} )
else ()
    message ( FATAL_ERROR "OpenGL not found! This will fail." )
endif ()

# add GL dir to include directories, so glu.h include is found
find_path(OpenglIncludeSubdir
            NAMES gl.h
            PATHS ${OPENGL_INCLUDE_DIR}
            PATH_SUFFIXES GL OpenGL
            NO_DEFAULT_PATH)
include_directories(${OpenglIncludeSubdir})

# GLEW
#
# doesn't work, we can't hint paths, would need to provide own findglew then
#if (WIN32)
#	find_package ( GLEW REQUIRED )
#	if ( GLEW_FOUND )
#	    message ( STATUS "GLEW found." )
#	    include_directories ( ${GLEW_INCLUDE_DIR} )
#	    set ( LIBS ${LIBS} ${GLEW_LIBRARY} )
#	else ()
#	    message ( FATAL_ERROR "GLEW not found! This will fail." )
#	endif ()
#endif ()


# SDL
#
find_package ( SDL REQUIRED )
if ( SDL_FOUND )
    message ( STATUS "SDL found." )
    include_directories ( ${SDL_INCLUDE_DIR} )
    set ( LIBS ${LIBS} ${SDL_LIBRARY} )
else ()
    message ( FATAL_ERROR "SDL not found! This will fail." )
endif ()


# SDL_image
#
find_package ( SDL_image REQUIRED )
if ( SDL_IMAGE_FOUND )
    message ( STATUS "SDL_image found." )
    include_directories ( ${SDL_IMAGE_INCLUDE_DIR} )
    set ( LIBS ${LIBS} ${SDL_IMAGE_LIBRARY} )
else ()
    message ( FATAL_ERROR "SDL_image not found! This will fail." )
endif ()


# SDL_mixer
#
find_package ( SDL_mixer REQUIRED )
if ( SDL_MIXER_FOUND )
    message ( STATUS "SDL_mixer found." )
    include_directories ( ${SDL_MIXER_INCLUDE_DIR} )
    set ( LIBS ${LIBS} ${SDL_MIXER_LIBRARY} )
else ()
    message ( FATAL_ERROR "SDL_mixer not found! This will fail." )
endif ()


# bzip2 package
#
find_package ( BZip2 REQUIRED )
if ( BZIP2_FOUND )
        message ( STATUS "BZip2 found." )
        include_directories ( ${BZIP2_INCLUDE_DIR} )
        set ( LIBS ${LIBS} ${BZIP2_LIBRARY} )
else ()
        message ( FATAL_ERROR "BZip2 not found! This will fail." )
endif ()

# check electric fence option and system presence
#
if ( USE_EFENCE )
    if ( UNIX )
        find_library ( EFENCE_LIBRARY NAMES efence
                                      DOC "The Electric Fence library"
                                      PATHS "/usr/lib"
                                            "/usr/lib64"
                                            "/usr/efence/lib"
                                            "/usr/share/efence/lib"
                                            )
        if ( EFENCE_LIBRARY )
            message ( STATUS "ElectricFence found and will linked against ${PROJECT_NAME}" )
            set ( LIBS ${LIBS} ${EFENCE_LIBRARY} )
        else ()
            message ( WARNING "ElectricFence not found! User option ignored." )
        endif ()
    else ()
        message ( WARNING "Not on UNIXes" )
    endif ()
endif ()


# FFMPEG for videoplaytest
#
if ( DFTD_BUILD_TOOLS AND DFTD_BUILD_VIDEO_TEST )
    find_package ( FFMPEG REQUIRED )
    if ( FFMPEG_FOUND )
        message ( STATUS "FFMPEG Found: " ${FFMPEG_INCLUDE_DIR} )
        include_directories ( ${FFMPEG_INCLUDE_DIR} )
        set ( LIBS ${LIBS} ${FFMPEG_LIBRARY} )
    else ()
        message ( WARNING "FFMPEG not found! Skipping videoplaytest utility." )
    endif ()
endif ()

