##############################################################################
# Looks for the SimD library installation and defines:
#
#  SimD_FOUND - system has SimD.
#  SimD_INCLUDE_DIRS - the SimD include directory.
#  SimD_LIBRARIES - Link these to use SimD.
#
# You need to define the SIMD_INSTALL_DIR variable or have the
# environment variable $SIMD_INSTALL_DIR to be set to your SimD
# installation directory.
##############################################################################

# If SIMD_INSTALL_DIR was defined in the environment, use it.
set(SIMD_INSTALL_DIR $ENV{OSPL_HOME}/src/api/dcps/isocpp/)
if (NOT SIMD_INSTALL_DIR AND NOT $ENV{SIMD_INSTALL_DIR} STREQUAL "")
set(SIMD_INSTALL_DIR $ENV{OSPL_HOME})
endif(NOT SIMD_INSTALL_DIR AND NOT $ENV{SIMD_INSTALL_DIR} STREQUAL "")

MESSAGE(SIMD: ${SIMD_INSTALL_DIR})
FIND_PATH(SimD_INCLUDE_DIR
NAMES
    dds/dds.hpp
PATHS
    ${SIMD_INSTALL_DIR}/include/
    MESSAGE(INSTALL DIR ${SIMD_INSTALL_DIR})
)

IF (SimD_INCLUDE_DIR)
    MESSAGE(STATUS "Found SimD include dir: ${SimD_INCLUDE_DIR}")
ELSE (SimD_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "Could not find SimD include dir")
ENDIF (SimD_INCLUDE_DIR)

SET(SimD_INCLUDE_DIRS
${SimD_INCLUDE_DIR}
)

FIND_LIBRARY(SimD_LIBRARY
    NAMES
        dcpsisocpp
    PATHS
    $ENV{OSPL_HOME}/lib/$ENV{SPLICE_TARGET}
)

SET(SimD_LIBRARIES
    ${SimD_LIBRARY}
)


IF (SimD_INCLUDE_DIRS AND SimD_LIBRARIES)
    SET(SimD_FOUND TRUE)
ENDIF (SimD_INCLUDE_DIRS AND SimD_LIBRARIES)

IF (SimD_FOUND)
    MESSAGE(STATUS "Found SimD C++ libraries: ${SimD_LIBRARIES}")
ELSE (SimD_FOUND)
    IF (SimD_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find SimD")
    ENDIF (SimD_FIND_REQUIRED)
ENDIF (SimD_FOUND)

MARK_AS_ADVANCED(SimD_INCLUDE_DIRS SimD_LIBRARIES)
