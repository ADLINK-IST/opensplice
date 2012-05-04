include ${OSPL_HOME}/custom_lib/ccpp/orbdeps.mak

#
# Include paths

INCLUDES+=-I./bld
INCLUDES+=-I../../../src
INCLUDES+=-I../../../../idl
INCLUDES+=-I../../../../../../common
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/CCPP
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/CCPP/$(SPLICE_ORB)
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/etc/idl
INCLUDES+=-I$(OSPL_HOME)/include/sys
INCLUDES+= $(ORB_INCLUDE)

# C++ Compiler settings.
CXX=g++
CXXFLAGS=-g -c -m64 -fPIC
CPPFLAGS=-D$(SPLICE_ORB) $(INCLUDES)

# Linker settings.
LD_SO=$(CXX)
LD_FLAGS=-fPIC
LD_LIBS=-lstdc++

#OpenSplice idl preprocessor
OSPLICE_COMP=$(OSPL_HOME)/bin/idlpp -C -l cpp -d bld

OSPLICE_LIBS=-lddsdatabase -ldcpsgapi -ldcpsccpp -lddsos

DCPS_SRC_FILES = bld/WaitSetDataC.cpp bld/WaitSetDataDcpsC.cpp bld/WaitSetDataDcps_impl.cpp bld/WaitSetDataSplDcps.cpp
DCPS_OBJ_FILES = $(patsubst %.cpp, %.o, $(DCPS_SRC_FILES))


bld/%.o : ../../../src/%.cpp
	@echo "Compiling $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all : dirs exec/WaitSetDataPublisher exec/WaitSetDataSubscriber
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : ../../../../idl/WaitSetData.idl
	@echo "Compiling $< with SPLICE IDL compiler"
	@mkdir -p bld
	@$(OSPLICE_COMP) $(INCLUDES) $<
	@echo "Compiling $< with TAO IDL compiler"
	@$(ORB_IDL_COMPILER) $(ORB_IDL_OUTPUT) bld $(INCLUDES) $(ORB_IDL_FLAGS) $<
	@@echo "Compiling $< Typed Interfaces with TAO IDL compiler"
	@$(ORB_IDL_COMPILER) $(ORB_IDL_OUTPUT) bld $(INCLUDES) $(ORB_IDL_FLAGS) bld/WaitSetDataDcps.idl


exec/WaitSetDataPublisher : $(DCPS_OBJ_FILES) bld/WaitSetDataPublisher.o bld/CheckStatus.o  bld/DDSEntityManager.o
	@echo "Linking WaitSetDataPublisher"
	@mkdir -p exec
	@$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(ORB_LDLIBS) $(LD_LIBS)

exec/WaitSetDataSubscriber : $(DCPS_OBJ_FILES) bld/WaitSetDataSubscriber.o bld/CheckStatus.o bld/DDSEntityManager.o
	@echo "Linking WaitSetDataSubscriber"
	@mkdir -p exec
	@$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(ORB_LDLIBS) $(LD_LIBS)

clean :
	@rm -f *.o
	@rm -rf bld
	@rm -rf exec


