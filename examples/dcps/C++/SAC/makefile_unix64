#
# Include paths
INCLUDES=-I./
INCLUDES+=-I./bld
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C/SAC
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/include/sys

# C++ Compiler settings.
CC=g++
CCFLAGS=-g -c -m64 -fPIC
CPPFLAGS=$(INCLUDES)

# Linker settings.
LD=$(CC)
LD_FLAGS=-fPIC
LD_LIBS=

#OpenSplice idl preprocessor
OSPLICE_COMP=$(OSPL_HOME)/bin/idlpp -S -l c -d bld

OSPLICE_LIBS=-lddsdatabase -ldcpssac

DCPS_SRC_FILES = bld/pingpongSacDcps.c  bld/pingpongSplDcps.c
DCPS_OBJ_FILES = $(patsubst %.c, %.o, $(DCPS_SRC_FILES))

.c.o :
	@echo "Compiling $<"
	$(CC) $(CPPFLAGS) $(CCFLAGS) $< -o $@

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
	$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

exec/pong : $(DCPS_OBJ_FILES) pong.o
	@echo "Linking PONG"
	@mkdir -p exec
	$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

clean :
	@rm -f *.o ospl-info.log
	@rm -f bld/*
	@rm -f exec/*
	@rmdir bld
	@rmdir exec
