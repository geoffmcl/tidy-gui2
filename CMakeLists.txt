# qt-gui project
# original CMakeLists.txt, generated from [src\qt-gui.pro]

cmake_minimum_required( VERSION 2.8.8 )

# CMakeScripts or use the ones that come by default with CMake.
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

project( Tidy-GUI2 )

# ### NOTE: *** Adjust version.txt when required ***
# read 'version' file into a variable (stripping any newlines or spaces)
file(READ version.txt versionFile)
if (NOT versionFile)
    message(FATAL_ERROR "Unable to determine libtidy version. version.txt file is missing.")
endif()
string(STRIP "${versionFile}" TIDYGUI2_VERSION)
string(REPLACE "." ";" VERSION_LIST ${TIDYGUI2_VERSION})
list(GET VERSION_LIST 0 TG2_MAJOR_VER)
list(GET VERSION_LIST 1 TG2_MINOR_VER)
list(GET VERSION_LIST 2 TG2_POINT_VER)

# This is the title bar displayed name and version
add_definitions( -DAPP_NAME="Tidy GUI 2" )
add_definitions( -DAPP_VERSION="${TG2_MAJOR_VER}.${TG2_MINOR_VER}.${TG2_POINT_VER}" )

# These should NOT be changed that often...
# It only set where the INI and default CFG file are
add_definitions( -DAPP_SETV="0.0.1" )
add_definitions( -DAPP_SETD="geoffair.org" )
add_definitions( -DAPP_SETO="geoffair" )
add_definitions( -DAPP_SETN="tidygui2" )

# Add LINUX or WINDOWS definitions
if(UNIX)
   add_definitions( -DLINUX )
else(UNIX)
   add_definitions( -DWINDOWS )
endif(UNIX)
if(CMAKE_COMPILER_IS_GNUCXX)
    set( WARNING_FLAGS -Wall )
endif(CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") 
   set( WARNING_FLAGS "-Wall -Wno-overloaded-virtual" )
endif() 

if(WIN32 AND MSVC)
    # turn off various warnings - none needed in this compile
    set(WARNING_FLAGS "${WARNING_FLAGS} /wd4996")
    # foreach(warning 4244 4251 4267 4275 4290 4786 4305)
    #     set(WARNING_FLAGS "${WARNING_FLAGS} /wd${warning}")
    # endforeach(warning)
    set( MSVC_FLAGS "-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D__CRT_NONSTDC_NO_WARNINGS" )
    # if (${MSVC_VERSION} EQUAL 1600)
    #    set( MSVC_LD_FLAGS "/FORCE:MULTIPLE" )
    # endif (${MSVC_VERSION} EQUAL 1600)
    #set( NOMINMAX 1 )
    # to distinguish between debug and release libs
    set( CMAKE_DEBUG_POSTFIX "d" )
else()
    # items for unix
    set(WARNING_FLAGS "${WARNING_FLAGS} -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-variable")
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MSVC_LD_FLAGS}" )

#  TIDY_FOUND          - True if tidy found.
#  TIDY_INCLUDE_DIRS   - where to find tidy.h, etc.
#  TIDY_LIBRARIES      - List of libraries when using tidy.
find_package( Tidy )
if (TIDY_FOUND)
    message(STATUS "*** Tidy found. lib ${TIDY_LIBRARIES} inc ${TIDY_INCLUDE_DIRS}")
    list(APPEND add_LIBS ${TIDY_LIBRARIES})
    include_directories( ${TIDY_INCLUDE_DIRS} )
else ()
    message(FATAL_ERROR "*** Tidy NOT found! Can NOT build Tidy GUI2")
endif ()

message(STATUS "*** Finding Qt4 components COMPONENTS QtCore QtGui QtXml QtXmlPatterns")
find_package ( Qt4 COMPONENTS QtCore QtGui QtXml QtXmlPatterns REQUIRED )
include ( ${QT_USE_FILE} )
add_definitions( ${QT_DEFINITIONS} )
# debug messages
message(STATUS "*** include ${QT_USE_FILE}")
message(STATUS "*** defs  ${QT_DEFINITIONS}")
message(STATUS "*** libs ${QT_LIBRARIES}")
list(APPEND add_LIBS ${QT_LIBRARIES})

# configuration file, if needed
# configure_file( ${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h )
# add_definitions( -DHAVE_CONFIG_H )
# include_directories( ${CMAKE_BINARY_DIR} )

# Added for DEBUG only
get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "*** inc_dirs = ${inc_dirs}")

##############################################
### Tidy GUI 2
set(name tidy-gui2)
set(dir src)
###  ${dir}/tg-scraps.cpp
set( ${name}_SRCS
    ${dir}/tg-main.cpp
    ${dir}/tg-dialog.cpp
    ${dir}/tg-config.cpp
    )
set( ${name}_HDRS
    ${dir}/tg-dialog.h
    ${dir}/tg-config.h
    )
set( ${name}_RCC
    ${dir}/icons/icons.qrc
    )
QT4_WRAP_CPP( ${name}_HDRS_MOC ${${name}_HDRS} )
QT4_ADD_RESOURCES( ${name}_RESOURCES_RCC ${${name}_RCC} )
add_executable( ${name} 
    ${${name}_SRCS} 
    ${${name}_HDRS_MOC} 
    ${${name}_HDRS} 
    ${${name}_RESOURCES_RCC} 
    )
target_link_libraries( ${name} ${QT_LIBRARIES} ${add_LIBS} )
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install 
# install(TARGETS qt-gui DESTINATION bin )

# eof
