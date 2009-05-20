
include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak

CINCS	:= -I.
CINCS += -I$(OSPL_HOME)/src/include
CINCS += -I$(OSPL_HOME)/src/abstraction/os/include
CINCS += -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/user/include

ifeq (,$(findstring vxworks,$(SPLICE_TARGET)))
LIBS	+= -l$(DDS_DATABASE) -l$(DDS_OS) -ldcpssac -l$(DDS_DCPSGAPI) -l$(DDS_USER) -l$(DDS_KERNEL) -l$(DDS_SERIALIZATION) -l$(DDS_UTIL) -l$(DDS_CONF) -l$(DDS_OS) -l$(DDS_CONFPARSER) -l$(DDS_DATABASE)
LIBS    += -lpthread -lm -lrt -ldl
endif

all link: ../../exec/$(SPLICE_TARGET)/ping ../../exec/$(SPLICE_TARGET)/pong

../../exec/$(SPLICE_TARGET)/ping: ping.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	filter_gcc $(CC) $(CFLAGS)  -o ../../exec/$(SPLICE_TARGET)/ping -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) \
		ping.o pingpongSacDcps.o pingpongSplDcps.o $(LIBS)

ping.o: pingpong.h ../../ping.c pingpongSacDcps.o pingpongSplDcps.o
	filter_gcc $(CC) $(CFLAGS)  -g $(CINCS) -c ../../ping.c
 
../../exec/$(SPLICE_TARGET)/pong: pong.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	filter_gcc $(CC) $(CFLAGS)  -o ../../exec/$(SPLICE_TARGET)/pong -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) \
		pong.o pingpongSacDcps.o pingpongSplDcps.o $(LIBS)

pong.o: pingpong.h ../../pong.c pingpongSacDcps.o pingpongSplDcps.o
	filter_gcc $(CC) $(CFLAGS)  -g $(CINCS) -c ../../pong.c

pingpongSacDcps.o: pingpongSacDcps.c pingpongDcps.h pingpongSacDcps.h
	filter_gcc $(CC) $(CFLAGS) -g $(CINCS) -c pingpongSacDcps.c 

pingpongSplDcps.o: pingpongSplDcps.c pingpongSplDcps.h pingpongDcps.h
	filter_gcc $(CC) $(CFLAGS) -g $(CINCS) -c pingpongSplDcps.c

pingpong.h pingpongDcps.h pingpongSacDcps.h pingpongSplDcps.h pingpongSacDcps.c pingpongSplDcps.c: ../../pingpong.idl
	idlpp -S -l c ../../pingpong.idl
