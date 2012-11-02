#
# Include paths
INCLUDES+=-I../../../idl
INCLUDES+=-I./bld
INCLUDES+=-I../../src
INCLUDES+=-I../../../../../common
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C/SAC
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/include/sys

# C Compiler settings.
CC=gcc
CCFLAGS=-g -c -m64 -fPIC
CPPFLAGS=$(INCLUDES)

# Linker settings.
LD=$(CC)
LD_FLAGS=-fPIC
LD_LIBS=

#OpenSplice idl preprocessor
OSPLICE_COMP=$(OSPL_HOME)/bin/idlpp -S -l c -d bld

OSPLICE_LIBS=-lddsdatabase -ldcpssac -ldcpsgapi -lddsos

DCPS_SRC_FILES = bld/ListenerDataSacDcps.c bld/ListenerDataSplDcps.c
DCPS_OBJ_FILES = $(patsubst %.c, %.o, $(DCPS_SRC_FILES))


bld/%.o : ../../src/%.c
	$(CC) $(CCFLAGS) $(CPPFLAGS) $< -o $@

all : dirs exec/ListenerDataPublisher exec/ListenerDataSubscriber
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : ../../../idl/ListenerData.idl
	@echo "Compiling $< with SPLICE IDL compiler"
	@mkdir -p bld
	@$(OSPLICE_COMP) $(INCLUDES) $<
	

exec/ListenerDataPublisher : $(DCPS_OBJ_FILES) bld/ListenerDataPublisher.o bld/CheckStatus.o  bld/DDSEntitiesManager.o
	@echo "Linking ListenerDataPublisher"
	@mkdir -p exec
	@$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)
exec/ListenerDataSubscriber : $(DCPS_OBJ_FILES) bld/ListenerDataSubscriber.o bld/ListenerDataListener.o bld/CheckStatus.o bld/DDSEntitiesManager.o
	@echo "Linking ListenerDataSubscriber"
	@mkdir -p exec
	@$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)	



clean :
	@rm -f *.o
	@rm -rf bld
	@rm -rf exec
	
	
