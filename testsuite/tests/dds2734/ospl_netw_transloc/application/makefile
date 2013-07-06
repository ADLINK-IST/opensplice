#
# Include paths
INCLUDES=-I./
INCLUDES+=-I./bld
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C/SAC
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/include/sys

# C++ Compiler settings.
CC=gcc
CCFLAGS=-g -c
CPPFLAGS=$(INCLUDES)

# Linker settings.
LD=$(CC)
LD_FLAGS=
LD_LIBS=

#OpenSplice idl preprocessor
OSPLICE_COMP=$(OSPL_HOME)/bin/idlpp -S -l c -d bld

OSPLICE_LIBS=-lddsdatabase -ldcpssac -lddsos

DCPS_SRC_FILES = bld/dds2734SacDcps.c  bld/dds2734SplDcps.c
DCPS_OBJ_FILES = $(patsubst %.c, %.o, $(DCPS_SRC_FILES))

.c.o :
	@echo "Compiling $<"
	@$(CC) $(CPPFLAGS) $(CCFLAGS) $< -o $@

all : dirs exec/dds2734_publisher exec/dds2734_subscriber
	@echo ">>>> all done"
	@echo ">>>> to execute, enter : sh RUN"
	@echo ">>>> to clean, enter : make clean"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : dds2734.idl
	@echo "Compiling IDL with SPLICE IDL compiler"
	@mkdir -p bld
	@$(OSPLICE_COMP) $(INCLUDES) $<

exec/dds2734_publisher : $(DCPS_OBJ_FILES) dds2734_publisher.o
	@echo "Linking pub"
	@mkdir -p exec
	@$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

exec/dds2734_subscriber : $(DCPS_OBJ_FILES) dds2734_subscriber.o
	@echo "Linking sub"
	@mkdir -p exec
	@$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)

clean :
	@rm -f *.o
	@rm -f bld/*
	@rm -f exec/*
	@rmdir bld
	@rmdir exec
