include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak

CXXINCS	:= -I./
CXXINCS += -I$(OSPL_HOME)/src/include
CXXINCS += -I$(OSPL_HOME)/src/abstraction/os/include
CXXINCS += -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
CXXINCS += -I$(OSPL_HOME)/src/database/database/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/ccpp/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/sacpp/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/sacpp/bld/$(SPLICE_TARGET)
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/gapi/include
CXXINCS += -I$(OSPL_HOME)/src/kernel/include
CXXINCS += -I$(OSPL_HOME)/src/user/include

all link: ../../exec/$(SPLICE_TARGET)/ping ../../exec/$(SPLICE_TARGET)/pong

../../exec/$(SPLICE_TARGET)/ping: ping.o pingpong.o pingpongDcps.o pingpongDcps_impl.o pingpongSplDcps.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	$(LD_EXE) -o ../../exec/$(SPLICE_TARGET)/ping -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) ping.o pingpong.o pingpongDcps.o pingpongDcps_impl.o pingpongSplDcps.o  -l$(DDS_DCPSSACPP) -l$(DDS_DATABASE) -l$(DDS_OS) $(LDLIBS_CXX)

ping.o: ../../ping.cpp pingpong.cpp pingpongDcps.h pingpong.h
	$(CXX) -g $(CXXINCS) -c ../../ping.cpp

../../exec/$(SPLICE_TARGET)/pong: pong.o pingpong.o pingpongDcps.o pingpongDcps_impl.o pingpongSplDcps.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	$(LD_EXE) -o  ../../exec/$(SPLICE_TARGET)/pong -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) pong.o pingpong.o pingpongDcps.o pingpongDcps_impl.o pingpongSplDcps.o  -l$(DDS_DCPSSACPP) -l$(DDS_DATABASE) -l$(DDS_OS) $(LDLIBS_CXX)

pong.o: ../../pong.cpp pingpong.cpp pingpongDcps.h pingpong.h
	$(CXX) -g $(CXXINCS) -c ../../pong.cpp

pingpong.o: pingpong.cpp
	$(CXX) -g $(CXXINCS) -c pingpong.cpp

pingpongDcps_impl.o: pingpongDcps_impl.cpp
	$(CXX) -g $(CXXINCS) -c pingpongDcps_impl.cpp

pingpongDcps.o: pingpongDcps.cpp
	$(CXX) -g $(CXXINCS) -c pingpongDcps.cpp

pingpongSplDcps.o: pingpongSplDcps.cpp
	$(CXX) -g $(CXXINCS) -c pingpongSplDcps.cpp

pingpongDcps.cpp pingpongDcps.h pingpongDcps.idl pingpong.h pingpong.cpp pingpongDcps_impl.h pingpongSplDcps.h pingpongDcps_impl.cpp pingpongSplDcps.cpp: ../../pingpong.idl
	idlpp -l cpp -S -I../.. -I$(OSPL_HOME)/etc/idl -I$(OSPL_HOME)/src/api/dcps/sacpp/bld/$(SPLICE_TARGET) ../../pingpong.idl

