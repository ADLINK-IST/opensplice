#
# Include paths
INCLUDES=-I./
INCLUDES+=-I./bld
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/SACPP
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/include/sys

# C++ Compiler settings.
CXX=g++
CXXFLAGS=-g -c -m64 -fPIC
CPPFLAGS=$(INCLUDES)

# Linker settings.
LD_SO=$(CXX)
LD_FLAGS=-fPIC
LD_LIBS=-lstdc++

#OpenSplice idl preprocessor
OSPLICE_COMP=$(OSPL_HOME)/bin/idlpp -S -l cpp -d bld

OSPLICE_LIBS=-lddsdatabase -ldcpsgapi -ldcpssacpp -lddsos

DCPS_SRC_FILES = bld/Chat.cpp  bld/ChatDcps.cpp  bld/ChatDcps_impl.cpp  bld/ChatSplDcps.cpp

DCPS_OBJ_FILES = $(patsubst %.cpp, %.o, $(DCPS_SRC_FILES))

.cpp.o :
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all : dirs exec/Chatter exec/MessageBoard exec/UserLoad
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : Chat.idl
	@echo "Compiling IDL with SPLICE IDL compiler"
	@mkdir -p bld
	$(OSPLICE_COMP) $(INCLUDES) $<

exec/Chatter : $(DCPS_OBJ_FILES) Chatter.o CheckStatus.o multitopic.o
	@echo "Linking Chatter"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

exec/MessageBoard : $(DCPS_OBJ_FILES) MessageBoard.o CheckStatus.o multitopic.o
	@echo "Linking MessageBoard"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

exec/UserLoad : $(DCPS_OBJ_FILES) UserLoad.o CheckStatus.o multitopic.o
	@echo "Linking UserLoad"
	@mkdir -p exec
	$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

clean :
	@rm -f *.o
	@rm -f bld/*
	@rm -f exec/*
	@rmdir bld
	@rmdir exec
