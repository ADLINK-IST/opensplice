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

DCPS_SRC_FILES = bld/ContentFilteredTopicDataSacDcps.c bld/ContentFilteredTopicDataSplDcps.c
DCPS_OBJ_FILES = $(patsubst %.c, %.o, $(DCPS_SRC_FILES))


bld/%.o : ../../src/%.c
	$(CC) $(CCFLAGS) $(CPPFLAGS) $< -o $@

all : dirs exec/ContentFilteredTopicDataPublisher exec/ContentFilteredTopicDataSubscriber
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec

$(DCPS_SRC_FILES) : ../../../idl/ContentFilteredTopicData.idl
	@echo "Compiling $< with SPLICE IDL compiler"
	@mkdir -p bld
	@$(OSPLICE_COMP) $(INCLUDES) $<
	

exec/ContentFilteredTopicDataPublisher : $(DCPS_OBJ_FILES) bld/ContentFilteredTopicDataPublisher.o bld/CheckStatus.o  bld/DDSEntitiesManager.o
	@echo "Linking ContentFilteredTopicDataPublisher"
	@mkdir -p exec
	@$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)
exec/ContentFilteredTopicDataSubscriber : $(DCPS_OBJ_FILES) bld/ContentFilteredTopicDataSubscriber.o bld/CheckStatus.o bld/DDSEntitiesManager.o
	@echo "Linking ContentFilteredTopicDataSubscriber"
	@mkdir -p exec
	@$(LD) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(LD_LIBS)	



clean :
	@rm -f *.o
	@rm -rf bld
	@rm -rf exec
	
	
