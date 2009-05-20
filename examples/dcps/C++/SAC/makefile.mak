
include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak

CXXINCS	:= -I./
CXXINCS += -I$(OSPL_HOME)/src/include
CXXINCS += -I$(OSPL_HOME)/src/abstraction/os/include
CXXINCS += -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
CXXINCS += -I$(OSPL_HOME)/src/database/database/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/sac/include
CXXINCS += -I$(OSPL_HOME)/src/kernel/include
CXXINCS += -I$(OSPL_HOME)/src/user/include

ifeq (,$(findstring vxworks,$(SPLICE_TARGET)))
LIBS	+= -ldcpssac
endif

all link: ../../exec/$(SPLICE_TARGET)/ping ../../exec/$(SPLICE_TARGET)/pong

../../exec/$(SPLICE_TARGET)/ping: ping.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	filter_gcc $(CXX) $(CFLAGS)  -o ../../exec/$(SPLICE_TARGET)/ping -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) \
		ping.o pingpongSacDcps.o pingpongSplDcps.o $(LIBS)

ping.o: pingpong.h ../../ping.cpp pingpongSacDcps.o pingpongSplDcps.o
	filter_gcc $(CC) $(CFLAGS)  -g $(CXXINCS) -c ../../ping.cpp
 
../../exec/$(SPLICE_TARGET)/pong: pong.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	filter_gcc $(CXX) $(CFLAGS)  -o ../../exec/$(SPLICE_TARGET)/pong -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) \
		pong.o pingpongSacDcps.o pingpongSplDcps.o $(LIBS)

pong.o: pingpong.h ../../pong.cpp pingpongSacDcps.o pingpongSplDcps.o
	filter_gcc $(CXX) $(CFLAGS)  -g $(CXXINCS) -c ../../pong.cpp

pingpongSacDcps.o: pingpongSacDcps.c pingpongDcps.h pingpongSacDcps.h
	filter_gcc $(CXX) $(CFLAGS) -g $(CXXINCS) -c pingpongSacDcps.c 

pingpongSplDcps.o: pingpongSplDcps.c pingpongSplDcps.h pingpongDcps.h
	filter_gcc $(CXX) $(CFLAGS) -g $(CXXINCS) -c pingpongSplDcps.c

pingpong.h pingpongDcps.h pingpongSacDcps.h pingpongSplDcps.h pingpongSacDcps.c pingpongSplDcps.c: ../../pingpong.idl
	idlpp -S -l c ../../pingpong.idl
