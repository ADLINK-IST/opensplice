include ${OSPL_HOME}/custom_lib/ccpp/orbdeps.mak

#
# Include paths
INCLUDES=-I./
INCLUDES+=-I./bld
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/CCPP
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/CCPP/$(SPLICE_ORB)
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/etc/idl
INCLUDES+=-I$(OSPL_HOME)/include/sys
INCLUDES+= $(ORB_INCLUDE)


# C++ Compiler settings.
CXX=g++
CXXFLAGS=-g -c -m32
CPPFLAGS=-D$(SPLICE_ORB) $(INCLUDES)

# Linker settings.
LD_SO=$(CXX)
LD_FLAGS=-m32
LD_LIBS=-lstdc++

#OpenSplice idl preprocessor
OSPLICE_COMP=$(OSPL_HOME)/bin/idlpp -C -l cpp -d bld

OSPLICE_LIBS=-lddsdatabase -ldcpsgapi -ldcpsccpp -lddsos

DCPS_SRC_FILES = bld/pingpongC.cpp bld/pingpongDcpsC.cpp bld/pingpongDcps_impl.cpp bld/pingpongSplDcps.cpp
DCPS_OBJ_FILES = $(patsubst %.cpp, %.o, $(DCPS_SRC_FILES))

.cpp.o :
	@echo "Compiling $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all : dirs exec/ping exec/pong
	@echo ">>>> all done"
	@echo ">>>> to execute, enter : sh RUN"
	@echo ">>>> to clean, enter : make clean"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : pingpong.idl
	@mkdir -p bld
	@echo "Compiling $< with SPLICE IDL compiler"
	@$(OSPLICE_COMP) $(INCLUDES) $<
	@echo "Compiling $< with TAO IDL compiler"
	@$(ORB_IDL_COMPILER) $(ORB_IDL_OUTPUT) bld $(INCLUDES) $(ORB_IDL_FLAGS) $<
	@@echo "Compiling IDL Typed Interfaces with TAO IDL compiler"
	@$(ORB_IDL_COMPILER) $(ORB_IDL_OUTPUT) bld $(INCLUDES) $(ORB_IDL_FLAGS) bld/pingpongDcps.idl

exec/ping : $(DCPS_OBJ_FILES) ping.o
	@echo "Linking PING"
	@mkdir -p exec
	@$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(ORB_LDLIBS) $(LD_LIBS)

exec/pong : $(DCPS_OBJ_FILES) pong.o
	@echo "Linking PONG"
	@mkdir -p exec
	@$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(ORB_LDLIBS) $(LD_LIBS)

clean :
	@rm -f *.o ospl-info.log
	@rm -f bld/*
	@rm -f exec/*
	@rmdir bld
	@rmdir exec
