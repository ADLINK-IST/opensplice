#######################
# Nombre del proyecto # 
#######################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_MODULE_PATH ../../cmake)
PROJECT (PRUEBA)

####################
# Chequear entorno #
####################
IF (WIN32)
	IF(DEBUG)
		SET (DEFINITIONS "-DNDEBUG")
	ENDIF(DEBUG)
ELSE (WIN32)
	IF(DEBUG)
		SET (DEFINITIONS "-ansi -g -pg -pipe -std=c++98 ")
	ELSE(DEBUG)
		IF(PEDANTIC)
			SET (DEFINITIONS "-ansi -g -pg -pipe -Wall -Wstrict-null-sentinel -Weffc++ -Wold-style-cast -pedantic ")
			SET (DEBUG 1)
		ELSE(PEDANTIC)
			SET (DEFINITIONS "-DNDEBUG -O3 -pipe ")
		ENDIF(PEDANTIC)
	ENDIF(DEBUG)
ENDIF (WIN32)

FIND_PACKAGE (OpenSplice REQUIRED)

###################
# Modelo de datos #
###################
SET (idls 
 datamodel.idl
)
# TODO: Extraer al fichero de macros.
FOREACH(idl ${idls})
	OpenSplice_IDLGEN (${idl})
	STRING (REGEX REPLACE "\(.*\).idl" "\\1.cpp" VARS_1 ${idl})
	STRING (REGEX REPLACE "\(.*\).idl" "\\1.h" VARS_2 ${idl})
	STRING (REGEX REPLACE "\(.*\).idl" "\\1Dcps.cpp" VARS_3 ${idl})
	STRING (REGEX REPLACE "\(.*\).idl" "\\1Dcps.h" VARS_4 ${idl})
	STRING (REGEX REPLACE "\(.*\).idl" "\\1Dcps_impl.cpp" VARS_5 ${idl})
	STRING (REGEX REPLACE "\(.*\).idl" "\\1Dcps_impl.h" VARS_6 ${idl})
	STRING (REGEX REPLACE "\(.*\).idl" "\\1SplDcps.cpp" VARS_7 ${idl})
	STRING (REGEX REPLACE "\(.*\).idl" "\\1SplDcps.h" VARS_8 ${idl})
	STRING (REGEX REPLACE "\(.*\).idl" "ccpp_\\1.h" VARS_9 ${idl})
	SET(OpenSplice_DATAMODEL ${OpenSplice_DATAMODEL} ${VARS_1} ${VARS_2} ${VARS_3} ${VARS_4} ${VARS_5} ${VARS_6} ${VARS_7} ${VARS_8} ${VARS_9})
ENDFOREACH(idl)

#############################
# Ficheros de codigo fuente #
#############################
SET (PUBLISHER_SOURCES
 publisher.cpp
)
SET (SUBSCRIBER_SOURCES
 subscriber.cpp
)

# Incluir directorios de cabeceras de las librerias a linkar.
INCLUDE_DIRECTORIES(
 ${OpenSplice_INCLUDE_DIRS}
)

# Incluir directorios con los binarios de las librerias a linkar.
#link_directories (
# ${LIBRARY_OUTPUT_PATH}
#)

######################
# Construir binarios #
######################
SET (PUBLISHER_EXE publisher)
SET (SUBSCRIBER_EXE subscriber)
SET (DATAMODEL datamodel)

ADD_EXECUTABLE (${PUBLISHER_EXE} ${PUBLISHER_SOURCES})
ADD_EXECUTABLE (${SUBSCRIBER_EXE} ${SUBSCRIBER_SOURCES})
ADD_LIBRARY (${DATAMODEL} SHARED ${OpenSplice_DATAMODEL})

ADD_DEFINITIONS (
 ${OpenSplice_DEFINITIONS}
 ${DEFINITIONS}
)

# Linkar contra librerias deseadas.
TARGET_LINK_LIBRARIES (${DATAMODEL} 
 ${OpenSplice_LIBRARIES}
)

TARGET_LINK_LIBRARIES (${PUBLISHER_EXE}
 ${OpenSplice_LIBRARIES}
 ${DATAMODEL} 
)

TARGET_LINK_LIBRARIES (${SUBSCRIBER_EXE}
 ${OpenSplice_LIBRARIES}
 ${DATAMODEL}
)
