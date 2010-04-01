include ${OSPL_HOME}/custom_lib/ccpp/orbdeps.mak

#
# Include paths
INCLUDES=-I./
INCLUDES+=-I./bld
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/CCPP
INCLUDES+=-I$(OSPL_HOME)/etc/idl
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/CCPP/$(SPLICE_ORB)
INCLUDES+="-I$(TAO_ROOT)/include"
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/include/sys

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

OSPLICE_LIBS=-lddsdatabase -ldcpsgapi -ldcpsccpp

DCPS_SRC_FILES = bld/ChatC.cpp  bld/ChatDcpsC.cpp  bld/ChatDcps_impl.cpp  bld/ChatSplDcps.cpp

DCPS_OBJ_FILES = $(patsubst %.cpp, %.o, $(DCPS_SRC_FILES))

.cpp.o :
	@echo "Compiling $<"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all : dirs exec/Chatter exec/MessageBoard exec/UserLoad
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : Chat.idl
	@echo "Compiling $< with SPLICE IDL compiler"
	@mkdir -p bld
	$(OSPLICE_COMP) $(INCLUDES) $<
	@echo "Compiling $< with TAO IDL compiler"
	$(ORB_IDL_COMPILER) $(ORB_IDL_OUTPUT) bld $(INCLUDES) $(ORB_IDL_FLAGS) $<
	@echo "Compiling DCPS IDL Typed Interfaces with TAO IDL compiler"
	$(ORB_IDL_COMPILER) $(ORB_IDL_OUTPUT) bld $(INCLUDES) $(ORB_IDL_FLAGS) bld/ChatDcps.idl

exec/Chatter : $(DCPS_OBJ_FILES) Chatter.o CheckStatus.o multitopic.o
	@echo "Linking Chatter"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(ORB_LDLIBS) $(LD_LIBS)

exec/MessageBoard : $(DCPS_OBJ_FILES) MessageBoard.o CheckStatus.o multitopic.o
	@echo "Linking MessageBoard"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(ORB_LDLIBS) $(LD_LIBS)

exec/UserLoad : $(DCPS_OBJ_FILES) UserLoad.o CheckStatus.o multitopic.o
	@echo "Linking UserLoad"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(ORB_LDLIBS) $(LD_LIBS)

clean :
	@rm -f *.o
	@rm -f bld/*
	@rm -f exec/*
	@rmdir bld
	@rmdir exec
