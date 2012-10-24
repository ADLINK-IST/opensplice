#
# Include paths
INCLUDES+=-I../../../idl
INCLUDES+=-I./bld
INCLUDES+=-I../../src
INCLUDES+=-I../../../../../common
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/SACPP
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/include/sys

# C++ Compiler settings.
CXX=g++
CXXFLAGS=-g -c
CPPFLAGS=$(INCLUDES)

# Linker settings.
LD_SO=$(CXX)
LD_FLAGS=
LD_LIBS=-lstdc++

#OpenSplice idl preprocessor
OSPLICE_COMP=$(OSPL_HOME)/bin/idlpp -S -l cpp -d bld

OSPLICE_LIBS=-lddsdatabase -ldcpsgapi -ldcpssacpp -lddsos

DCPS_SRC_FILES = bld/ListenerData.cpp bld/ListenerDataDcps_impl.cpp bld/ListenerDataDcps.cpp bld/ListenerDataSplDcps.cpp
DCPS_OBJ_FILES = $(patsubst %.cpp, %.o, $(DCPS_SRC_FILES))


bld/%.o : ../../src/%.cpp
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all : dirs exec/ListenerDataPublisher exec/ListenerDataSubscriber
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : ../../../idl/ListenerData.idl
	@echo "Compiling $< with SPLICE IDL compiler"
	@mkdir -p bld
	@$(OSPLICE_COMP) $(INCLUDES) $<
	

exec/ListenerDataPublisher : $(DCPS_OBJ_FILES) bld/ListenerDataPublisher.o bld/CheckStatus.o  bld/DDSEntityManager.o
	@echo "Linking ListenerDataPublisher"
	@mkdir -p exec
	@$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

exec/ListenerDataSubscriber : $(DCPS_OBJ_FILES) bld/ListenerDataSubscriber.o bld/ListenerDataListener.o bld/CheckStatus.o bld/DDSEntityManager.o
	@echo "Linking ListenerDataSubscriber"
	@mkdir -p exec
	@$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)


clean :
	@rm -f *.o
	@rm -rf bld
	@rm -rf exec
	
	
