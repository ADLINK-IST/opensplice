
include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak
#include $(OSPL_HOME)/setup/makefiles/orbdeps_sa.mak

INCLUDE	:= -I./
#INCLUDE += $(ORB_SA_INCLUDE)
INCLUDE += -I$(OSPL_HOME)/src/include
INCLUDE += -I$(OSPL_HOME)/src/abstraction/os/include
INCLUDE += -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
INCLUDE += -I$(OSPL_HOME)/src/database/database/include
INCLUDE += -I$(OSPL_HOME)/src/api/dcps/ccpp/include
INCLUDE += -I$(OSPL_HOME)/src/api/dcps/sacpp/include
INCLUDE += -I$(OSPL_HOME)/src/api/dcps/sacpp/bld/$(SPLICE_TARGET)
INCLUDE += -I$(OSPL_HOME)/src/api/dcps/gapi/include
INCLUDE += -I$(OSPL_HOME)/src/kernel/include
INCLUDE += -I$(OSPL_HOME)/src/user/include

all link: ../../exec/$(SPLICE_TARGET)/ping ../../exec/$(SPLICE_TARGET)/pong

../../exec/$(SPLICE_TARGET)/ping: ping.o pingpong.o pingpongDcps.o pingpongDcps_impl.o pingpongSplDcps.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	$(LD_EXE) -o ../../exec/$(SPLICE_TARGET)/ping -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) ping.o pingpong.o pingpongDcps.o pingpongDcps_impl.o pingpongSplDcps.o  -l$(DDS_DCPSSACPP) -l$(DDS_DATABASE) -l$(DDS_OS) $(LDLIBS_CXX)

ping.o: ../../ping.cpp pingpong.cpp pingpongDcps.h pingpong.h
	$(CXX) -g $(INCLUDE) -c ../../ping.cpp
 
../../exec/$(SPLICE_TARGET)/pong: pong.o pingpong.o pingpongDcps.o pingpongDcps_impl.o pingpongSplDcps.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	$(LD_EXE) -o  ../../exec/$(SPLICE_TARGET)/pong -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) pong.o pingpong.o pingpongDcps.o pingpongDcps_impl.o pingpongSplDcps.o  -l$(DDS_DCPSSACPP) -l$(DDS_DATABASE) -l$(DDS_OS) $(LDLIBS_CXX)

pong.o: ../../pong.cpp pingpong.cpp pingpongDcps.h pingpong.h
	$(CXX) -g $(INCLUDE) -c ../../pong.cpp

pingpong.o: pingpong.cpp 
	$(CXX) -g $(INCLUDE) -c pingpong.cpp

pingpongDcps_impl.o: pingpongDcps_impl.cpp 
	$(CXX) -g $(INCLUDE) -c pingpongDcps_impl.cpp 

pingpongDcps.o: pingpongDcps.cpp 
	$(CXX) -g $(INCLUDE) -c pingpongDcps.cpp 

pingpongSplDcps.o: pingpongSplDcps.cpp 
	$(CXX) -g $(INCLUDE) -c pingpongSplDcps.cpp

pingpongDcps.cpp pingpongDcps.h pingpongDcps.idl pingpong.h pingpong.cpp pingpongDcps_impl.h pingpongSplDcps.h pingpongDcps_impl.cpp pingpongSplDcps.cpp: ../../pingpong.idl
	idlpp -l cpp -S -I../.. -I$(OSPL_HOME)/src/api/dcps/ccpp/idl -I$(OSPL_HOME)/src/api/dcps/sacpp/bld/$(SPLICE_TARGET) ../../pingpong.idl

