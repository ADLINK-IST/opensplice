include ${OSPL_HOME}/custom_lib/ccpp/orbdeps.mak

#
# Include paths

INCLUDES+=-I./bld
INCLUDES+=-I../../../src
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/CCPP
INCLUDES+=-I$(OSPL_HOME)/include/dcps/C++/CCPP/$(SPLICE_ORB)
INCLUDES+=-I$(OSPL_HOME)/include
INCLUDES+=-I$(OSPL_HOME)/etc/idl
INCLUDES+=-I$(OSPL_HOME)/include/sys
INCLUDES+= $(ORB_INCLUDE)

# C++ Compiler settings.
CXX=g++
CXXFLAGS=-g -c -m64 -fPIC
CPPFLAGS=-D$(SPLICE_ORB) $(INCLUDES)

# Linker settings.
LD_SO=$(CXX)
LD_FLAGS=-fPIC
LD_LIBS=-lstdc++

OSPLICE_LIBS=-lddsdatabase -ldcpsgapi -ldcpsccpp -lddsos


bld/%.o : ../../../src/%.cpp
	@echo "Compiling $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all : dirs exec/BuiltInTopicsDataSubscriber
	@echo ">>>> all done"

dirs :
	@mkdir -p bld
	@mkdir -p exec

exec/BuiltInTopicsDataSubscriber : $(DCPS_OBJ_FILES) bld/BuiltInTopicsDataSubscriber.o bld/CheckStatus.o bld/DDSEntityManager.o
	@echo "Linking BuiltInTopicsDataSubscriber"
	@mkdir -p exec
	@$(LD_SO) $(LD_FLAGS) -L$(OSPL_HOME)/lib -o $@ $^ $(OSPLICE_LIBS) $(ORB_LDLIBS) $(LD_LIBS)

clean :
	@rm -f *.o
	@rm -rf bld
	@rm -rf exec


