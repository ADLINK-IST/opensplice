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

OSPLICE_LIBS=-lddsdatabase -ldcpssac -ldcpsgapi -lddsos


bld/%.o : ../../src/%.c
	$(CC) $(CCFLAGS) $(CPPFLAGS) $< -o $@

all : dirs exec/BuiltInTopicsDataSubscriber
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec


exec/BuiltInTopicsDataSubscriber : $(DCPS_OBJ_FILES) bld/BuiltInTopicsDataSubscriber.o bld/CheckStatus.o bld/DDSEntitiesManager.o
	@echo "Linking BuiltInTopicsDataSubscriber"
	@mkdir -p exec
	@$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)	



clean :
	@rm -f *.o
	@rm -rf bld
	@rm -rf exec
	
	
