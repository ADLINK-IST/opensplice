#
# Include paths
INCLUDES=-I./
INCLUDES+=-I./bld
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/include/sys
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/SACPP
INCLUDES+=-I$(OSPL_HOME)/etc/idl

# C++ Compiler settings.
CXX=g++
CXXFLAGS=-g -c -m32
CPPFLAGS=$(INCLUDES) -I$(OSPL_HOME)/include/dcps/C++/SACPP

# Linker settings.
LD_SO=$(CXX)
LD_FLAGS=-m32
LD_LIBS=-lstdc++

#OpenSplice idl preprocessor
OSPLICE_COMP=$(OSPL_HOME)/bin/idlpp -S -l cpp -d bld

OSPLICE_LIBS=-lddsdatabase -ldcpsgapi -ldcpssacpp -lddsos

DCPS_SRC_FILES = bld/pingpong.cpp bld/pingpongDcps.cpp bld/pingpongDcps_impl.cpp bld/pingpongSplDcps.cpp
DCPS_OBJ_FILES = $(patsubst %.cpp, %.o, $(DCPS_SRC_FILES))

.cpp.o :
	@echo "Compiling $<"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all : dirs exec/ping exec/pong
	@echo ">>>> all done"
	@echo ">>>> to execute, enter : sh RUN"
	@echo ">>>> to clean, enter : make clean"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : pingpong.idl
	@echo "Compiling $< with SPLICE IDL compiler"
	@mkdir -p bld
	$(OSPLICE_COMP) $(INCLUDES) $<

exec/ping : $(DCPS_OBJ_FILES) ping.o
	@echo "Linking PING"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

exec/pong : $(DCPS_OBJ_FILES) pong.o
	@echo "Linking PONG"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

clean :
	@rm -f *.o ospl-info.log
	@rm -f bld/*
	@rm -f exec/*
	@rmdir bld
	@rmdir exec
