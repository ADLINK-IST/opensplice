include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak
include $(OSPL_HOME)/setup/makefiles/orbdeps.mak

CXXINCS	:= -I./
CXXINCS += $(ORB_INCLUDE)
CXXINCS += -I$(OSPL_HOME)/src/include
CXXINCS += -I$(OSPL_HOME)/src/abstraction/os/include
CXXINCS += -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
CXXINCS += -I$(OSPL_HOME)/src/database/database/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/ccpp/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/ccpp/orb/$(SPLICE_ORB)
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/ccpp/bld/$(SPLICE_TARGET)
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/gapi/include
CXXINCS += -I$(OSPL_HOME)/src/kernel/include
CXXINCS += -I$(OSPL_HOME)/src/user/include

all link: ../../exec/$(SPLICE_TARGET)/ping ../../exec/$(SPLICE_TARGET)/pong

../../exec/$(SPLICE_TARGET)/ping: ping.o pingpongC.o pingpongDcpsC.o pingpongDcps_impl.o pingpongSplDcps.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	$(LD_EXE) -o ../../exec/$(SPLICE_TARGET)/ping -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) ping.o pingpongC.o pingpongDcpsC.o pingpongDcps_impl.o pingpongSplDcps.o $(ORB_LDLIBS) -l$(DDS_DCPSCCPP) -lstdc++

ping.o: ../../ping.cpp pingpongC.cpp pingpongDcpsC.h pingpong.h
	$(CXX) -g $(CXXINCS) -c ../../ping.cpp

../../exec/$(SPLICE_TARGET)/pong: pong.o pingpongC.o pingpongDcpsC.o pingpongDcps_impl.o pingpongSplDcps.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	$(LD_EXE) -o  ../../exec/$(SPLICE_TARGET)/pong -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) pong.o pingpongC.o pingpongDcpsC.o pingpongDcps_impl.o pingpongSplDcps.o $(ORB_LDLIBS) -l$(DDS_DCPSCCPP) -lstdc++

pong.o: ../../pong.cpp pingpongC.cpp pingpongDcpsC.h pingpong.h
	$(CXX) -g  $(CXXINCS) -c ../../pong.cpp

pingpongC.o: pingpongC.cpp
	$(CXX) -g  -I./ $(CXXINCS) -c pingpongC.cpp

pingpongDcps_impl.o: pingpongDcps_impl.cpp
	$(CXX) -g  $(CXXINCS) -c pingpongDcps_impl.cpp

pingpongDcpsC.o: pingpongDcpsC.cpp
	$(CXX) -g  $(CXXINCS) -c pingpongDcpsC.cpp

pingpongSplDcps.o: pingpongSplDcps.cpp
	$(CXX) -g  $(CXXINCS) -c pingpongSplDcps.cpp

pingpongDcps.idl pingpong.h pingpongDcps_impl.h pingpongSplDcps.h pingpongDcps_impl.cpp pingpongSplDcps.cpp pingpongC.cpp pingpongC.h: ../../pingpong.idl
	$(ORB_COMPILER) -I../../ -I$(OSPL_HOME)/etc/idl $(ORB_CXXFLAGS) ../../pingpong.idl
	idlpp -C -l cpp ../../pingpong.idl

pingpongDcpsC.cpp pingpongDcpsC.h : pingpongDcps.idl
	$(ORB_COMPILER) -I../../ -I$(OSPL_HOME)/etc/idl $(ORB_CXXFLAGS) pingpongDcps.idl

