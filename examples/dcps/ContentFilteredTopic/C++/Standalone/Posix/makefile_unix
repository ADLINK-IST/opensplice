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

DCPS_SRC_FILES = bld/ContentFilteredTopicData.cpp bld/ContentFilteredTopicDataDcps_impl.cpp bld/ContentFilteredTopicDataDcps.cpp bld/ContentFilteredTopicDataSplDcps.cpp
DCPS_OBJ_FILES = $(patsubst %.cpp, %.o, $(DCPS_SRC_FILES))


bld/%.o : ../../src/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all : dirs exec/ContentFilteredTopicDataPublisher exec/ContentFilteredTopicDataSubscriber
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : ../../../idl/ContentFilteredTopicData.idl
	@echo "Compiling $< with SPLICE IDL compiler"
	@mkdir -p bld
	@$(OSPLICE_COMP) $(INCLUDES) $<


exec/ContentFilteredTopicDataPublisher : $(DCPS_OBJ_FILES) bld/ContentFilteredTopicDataPublisher.o bld/CheckStatus.o  bld/DDSEntityManager.o
	@echo "Linking ContentFilteredTopicDataPublisher"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

exec/ContentFilteredTopicDataSubscriber : $(DCPS_OBJ_FILES) bld/ContentFilteredTopicDataSubscriber.o bld/CheckStatus.o bld/DDSEntityManager.o
	@echo "Linking ContentFilteredTopicDataSubscriber"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)	



clean :
	@rm -f *.o
	@rm -rf bld
	@rm -rf exec
	
	
