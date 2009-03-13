include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak
include $(OSPL_HOME)/setup/makefiles/orbdeps.mak

INCLUDE	:= -I./
INCLUDE += $(ORB_INCLUDE)
INCLUDE += -I$(OSPL_HOME)/src/include
INCLUDE += -I$(OSPL_HOME)/src/abstraction/os/include
INCLUDE += -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
INCLUDE += -I$(OSPL_HOME)/src/database/database/include
INCLUDE += -I$(OSPL_HOME)/src/api/dcps/ccpp/include
INCLUDE += -I$(OSPL_HOME)/src/api/dcps/ccpp/orb/$(SPLICE_ORB)
INCLUDE += -I$(OSPL_HOME)/src/api/dcps/ccpp/bld/$(SPLICE_TARGET)
INCLUDE += -I$(OSPL_HOME)/src/api/dcps/gapi/include
INCLUDE += -I$(OSPL_HOME)/src/kernel/include
INCLUDE += -I$(OSPL_HOME)/src/user/include

all link: ../../exec/$(SPLICE_TARGET)/ping ../../exec/$(SPLICE_TARGET)/pong

../../exec/$(SPLICE_TARGET)/ping: ping.o pingpongC.o pingpongDcpsC.o pingpongDcps_impl.o pingpongSplDcps.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	$(LD_EXE) -o ../../exec/$(SPLICE_TARGET)/ping -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) ping.o pingpongC.o pingpongDcpsC.o pingpongDcps_impl.o pingpongSplDcps.o $(ORB_LDLIBS) -l$(DDS_DCPSCCPP) -lstdc++

ping.o: ../../ping.cpp pingpongC.cpp pingpongDcpsC.h pingpong.h
	$(CXX) -g $(INCLUDE) -c ../../ping.cpp
 
../../exec/$(SPLICE_TARGET)/pong: pong.o pingpongC.o pingpongDcpsC.o pingpongDcps_impl.o pingpongSplDcps.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	$(LD_EXE) -o  ../../exec/$(SPLICE_TARGET)/pong -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) pong.o pingpongC.o pingpongDcpsC.o pingpongDcps_impl.o pingpongSplDcps.o $(ORB_LDLIBS) -l$(DDS_DCPSCCPP) -lstdc++

pong.o: ../../pong.cpp pingpongC.cpp pingpongDcpsC.h pingpong.h
	$(CXX) -g  $(INCLUDE) -c ../../pong.cpp

pingpongC.o: pingpongC.cpp 
	$(CXX) -g  -I./ $(ORB_INCLUDE) -c pingpongC.cpp

pingpongDcps_impl.o: pingpongDcps_impl.cpp 
	$(CXX) -g  $(INCLUDE) -c pingpongDcps_impl.cpp 

pingpongDcpsC.o: pingpongDcpsC.cpp 
	$(CXX) -g  $(INCLUDE) -c pingpongDcpsC.cpp 

pingpongSplDcps.o: pingpongSplDcps.cpp 
	$(CXX) -g  $(INCLUDE) -c pingpongSplDcps.cpp

pingpongDcps.idl pingpong.h pingpongDcps_impl.h pingpongSplDcps.h pingpongDcps_impl.cpp pingpongSplDcps.cpp pingpongC.cpp pingpongC.h: ../../pingpong.idl
	$(ORB_COMPILER) -I../../ -I$(OSPL_HOME)/src/api/dcps/ccpp/idl $(ORB_CXXFLAGS) ../../pingpong.idl
	idlpp -C -l cpp ../../pingpong.idl

pingpongDcpsC.cpp pingpongDcpsC.h : pingpongDcps.idl
	$(ORB_COMPILER) -I../../ -I$(OSPL_HOME)/src/api/dcps/ccpp/idl $(ORB_CXXFLAGS) pingpongDcps.idl
    
