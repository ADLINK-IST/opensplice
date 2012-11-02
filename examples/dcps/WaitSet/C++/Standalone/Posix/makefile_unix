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

DCPS_SRC_FILES = bld/WaitSetData.cpp bld/WaitSetDataDcps_impl.cpp bld/WaitSetDataDcps.cpp bld/WaitSetDataSplDcps.cpp
DCPS_OBJ_FILES = $(patsubst %.cpp, %.o, $(DCPS_SRC_FILES))


bld/%.o : ../../src/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all : dirs exec/WaitSetDataPublisher exec/WaitSetDataSubscriber
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : ../../../idl/WaitSetData.idl
	@echo "Compiling $< with SPLICE IDL compiler"
	@mkdir -p bld
	@$(OSPLICE_COMP) $(INCLUDES) $<
	

exec/WaitSetDataPublisher : $(DCPS_OBJ_FILES) bld/WaitSetDataPublisher.o bld/CheckStatus.o  bld/DDSEntityManager.o
	@echo "Linking WaitSetDataPublisher"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)
	
exec/WaitSetDataSubscriber : $(DCPS_OBJ_FILES) bld/WaitSetDataSubscriber.o bld/CheckStatus.o bld/DDSEntityManager.o
	@echo "Linking WaitSetDataSubscriber"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)	



clean :
	@rm -f *.o
	@rm -rf bld
	@rm -rf exec
	
	
