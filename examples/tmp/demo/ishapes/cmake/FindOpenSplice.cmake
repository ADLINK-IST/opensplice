##############################################################################
# Try to find OpenSplice
# Once done this will define:
#
#  OpenSplice_FOUND - system has OpenSplice.
#  OpenSplice_INCLUDE_DIRS - the OpenSplice include directory.
#  OpenSplice_LIBRARIES - Link these to use OpenSplice.
#  OpenSplice_IDLGEN_BINARY - Binary for the IDL compiler.
#
# You need the environment variable $OSPL_HOME to be set to your OpenSplice
# installation directory.
# This script also includes the MacroOpenSplice.cmake script, which is useful
# for generating code from your idl.
#
##############################################################################
# Courtesy of Ivan Galvez Junquera <ivgalvez@gmail.com>
##############################################################################
SET(CMAKE_VERBOSE_MAKEFILE ON)
FIND_PATH(OpenSplice_INCLUDE_DIR
NAMES
ccpp_dds_dcps.h
#dds_dcps.idl
PATHS
$ENV{OSPL_HOME}/install/HDE/$ENV{SPLICE_TARGET}/include/dcps/C++/SACPP
#$ENV{OSPL_HOME}/etc/idl/
)

SET(OpenSplice_INCLUDE_DIRS
${OpenSplice_INCLUDE_DIR}
$ENV{OSPL_HOME}/install/HDE/$ENV{SPLICE_TARGET}/include
$ENV{OSPL_HOME}/install/HDE/$ENV{SPLICE_TARGET}/include/sys
$ENV{OSPL_HOME}/etc/idl/
)

# Find libraries
FIND_LIBRARY(DCPSGAPI_LIBRARY
NAMES
dcpsgapi
PATHS
$ENV{OSPL_HOME}/lib/$ENV{SPLICE_TARGET}
)

FIND_LIBRARY(DCPSSACPP_LIBRARY
NAMES
dcpssacpp
PATHS
$ENV{OSPL_HOME}/lib/$ENV{SPLICE_TARGET}
)

FIND_LIBRARY(DDSDATABASE_LIBRARY
NAMES
ddsdatabase
PATHS
$ENV{OSPL_HOME}/lib/$ENV{SPLICE_TARGET}
)

FIND_LIBRARY(DDSOS_LIBRARY
NAMES
ddsos
PATHS
$ENV{OSPL_HOME}/lib/$ENV{SPLICE_TARGET}
)

SET(OpenSplice_LIBRARIES
${DCPSGAPI_LIBRARY}
${DCPSSACPP_LIBRARY}
${DDSDATABASE_LIBRARY}
${DDSOS_LIBRARY}
)

# Binary for the IDL compiler
SET (OpenSplice_IDLGEN_BINARY $ENV{OSPL_HOME}/exec/$ENV{SPLICE_TARGET}/idlpp -I $ENV{OSPL_HOME}/etc/idl/)

IF (OpenSplice_INCLUDE_DIRS AND OpenSplice_LIBRARIES)
SET(OpenSplice_FOUND TRUE)
ENDIF (OpenSplice_INCLUDE_DIRS AND OpenSplice_LIBRARIES)

IF (OpenSplice_FOUND)
MESSAGE(STATUS "Found OpenSplice DDS libraries: ${OpenSplice_LIBRARIES}")
ELSE (OpenSplice_FOUND)
IF (OpenSplice_FIND_REQUIRED)
MESSAGE(FATAL_ERROR "Could not find OpenSplice DDS")
ENDIF (OpenSplice_FIND_REQUIRED)
ENDIF (OpenSplice_FOUND)

#MARK_AS_ADVANCED(OpenSplice_INCLUDE_DIRS OpenSplice_LIBRARIES OpenSplice_IDLGEN_BINARY)
INCLUDE (MacroOpenSplice)
