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
CXXFLAGS=-g -c -m32
CPPFLAGS=-D$(SPLICE_ORB) $(INCLUDES)

# Linker settings.
LD_SO=$(CXX)
LD_FLAGS=-m32
LD_LIBS=-lstdc++

#OpenSplice idl preprocessor
OSPLICE_COMP=$(OSPL_HOME)/bin/idlpp -C -l cpp -d bld

OSPLICE_LIBS=-lddsdatabase -ldcpsgapi -ldcpsccpp -lddsos

DCPS_SRC_FILES = bld/DurabilityDataC.cpp bld/DurabilityDataDcpsC.cpp bld/DurabilityDataDcps_impl.cpp bld/DurabilityDataSplDcps.cpp
DCPS_OBJ_FILES = $(patsubst %.cpp, %.o, $(DCPS_SRC_FILES))


bld/%.o : ../../../src/%.cpp
	@echo "Compiling $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all : dirs exec/DurabilityDataPublisher exec/DurabilityDataSubscriber
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : ../../../../idl/DurabilityData.idl
	@echo "Compiling $< with SPLICE IDL compiler"
	@mkdir -p bld
	@$(OSPLICE_COMP) $(INCLUDES) $<
	@echo "Compiling $< with TAO IDL compiler"
	@$(ORB_IDL_COMPILER) $(ORB_IDL_OUTPUT) bld $(INCLUDES) $(ORB_IDL_FLAGS) $<
	@@echo "Compiling $< Typed Interfaces with TAO IDL compiler"
	@$(ORB_IDL_COMPILER) $(ORB_IDL_OUTPUT) bld $(INCLUDES) $(ORB_IDL_FLAGS) bld/DurabilityDataDcps.idl
	

exec/DurabilityDataPublisher : $(DCPS_OBJ_FILES) bld/DurabilityDataPublisher.o bld/CheckStatus.o  bld/DDSEntityManager.o
	@echo "Linking DurabilityDataPublisher"
	@mkdir -p exec
	@$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(ORB_LDLIBS) $(LD_LIBS)

exec/DurabilityDataSubscriber : $(DCPS_OBJ_FILES) bld/DurabilityDataSubscriber.o bld/CheckStatus.o bld/DDSEntityManager.o
	@echo "Linking DurabilityDataSubscriber"
	@mkdir -p exec
	@$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(ORB_LDLIBS) $(LD_LIBS)


clean :
	@rm -f *.o
	@rm -rf bld
	@rm -rf exec
	
	
