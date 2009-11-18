#
# Include paths
INCLUDES=-I./
INCLUDES+=-I./bld
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C/SAC
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/include/sys

# C++ Compiler settings.
CC=gcc
CCFLAGS=-g -c -m32
CPPFLAGS=$(INCLUDES)

# Linker settings.
LD=$(CC)
LD_FLAGS=-m32
LD_LIBS=

#OpenSplice idl preprocessor
OSPLICE_COMP=$(OSPL_HOME)/bin/idlpp -S -l c -d bld

OSPLICE_LIBS=-lddsdatabase -ldcpssac

DCPS_SRC_FILES = bld/ChatSacDcps.c  bld/ChatSplDcps.c

DCPS_OBJ_FILES = $(patsubst %.c, %.o, $(DCPS_SRC_FILES))

.c.o :
	$(CC) $(CPPFLAGS) $(CCFLAGS) $< -o $@

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
	$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

exec/MessageBoard : $(DCPS_OBJ_FILES) MessageBoard.o CheckStatus.o multitopic.o
	@echo "Linking MessageBoard"
	@mkdir -p exec
	$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

exec/UserLoad : $(DCPS_OBJ_FILES) UserLoad.o CheckStatus.o multitopic.o
	@echo "Linking UserLoad"
	@mkdir -p exec
	$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

clean :
	@rm -f *.o
	@rm -f bld/*
	@rm -f exec/*
	@rmdir bld
	@rmdir exec
