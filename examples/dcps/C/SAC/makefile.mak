
include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak

INCLUDE	:= -I.
INCLUDE += -I$(OSPL_HOME)/src/include
INCLUDE += -I$(OSPL_HOME)/src/abstraction/os/include
INCLUDE += -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
INCLUDE += -I$(OSPL_HOME)/src/database/database/include
INCLUDE += -I$(OSPL_HOME)/src/api/dcps/sac/include
INCLUDE += -I$(OSPL_HOME)/src/kernel/include
INCLUDE += -I$(OSPL_HOME)/src/user/include

ifeq (,$(findstring vxworks,$(SPLICE_TARGET)))
LIBS	+= -l$(DDS_DATABASE) -ldcpssac
endif

all link: ../../exec/$(SPLICE_TARGET)/ping ../../exec/$(SPLICE_TARGET)/pong

../../exec/$(SPLICE_TARGET)/ping: ping.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	filter_gcc $(CC) $(CFLAGS)  -o ../../exec/$(SPLICE_TARGET)/ping -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) \
		ping.o pingpongSacDcps.o pingpongSplDcps.o $(LIBS)

ping.o: pingpong.h ../../ping.c pingpongSacDcps.o pingpongSplDcps.o
	filter_gcc $(CC) $(CFLAGS)  -g $(INCLUDE) -c ../../ping.c
 
../../exec/$(SPLICE_TARGET)/pong: pong.o
	mkdir -p ../../exec/$(SPLICE_TARGET)
	filter_gcc $(CC) $(CFLAGS)  -o ../../exec/$(SPLICE_TARGET)/pong -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) \
		pong.o pingpongSacDcps.o pingpongSplDcps.o $(LIBS)

pong.o: pingpong.h ../../pong.c pingpongSacDcps.o pingpongSplDcps.o
	filter_gcc $(CC) $(CFLAGS)  -g $(INCLUDE) -c ../../pong.c

pingpongSacDcps.o: pingpongSacDcps.c pingpongDcps.h pingpongSacDcps.h
	filter_gcc $(CC) $(CFLAGS) -g $(INCLUDE) -c pingpongSacDcps.c 

pingpongSplDcps.o: pingpongSplDcps.c pingpongSplDcps.h pingpongDcps.h
	filter_gcc $(CC) $(CFLAGS) -g $(INCLUDE) -c pingpongSplDcps.c

pingpong.h pingpongDcps.h pingpongSacDcps.h pingpongSplDcps.h pingpongSacDcps.c pingpongSplDcps.c: ../../pingpong.idl
	idlpp -S -l c ../../pingpong.idl
