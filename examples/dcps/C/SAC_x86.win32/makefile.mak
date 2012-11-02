
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
LIBS	+= -l$(DDS_DATABASE) -l$(DDS_OS) -l$(DDS_DCPSSAC) -l$(DDS_DCPSGAPI) -l$(DDS_USER) -l$(DDS_KERNEL)
#LIBS    += -lpthread -lm -lrt -ldl
endif

all link: ../../exec/$(SPLICE_TARGET)/ping.exe ../../exec/$(SPLICE_TARGET)/pong.exe

../../exec/$(SPLICE_TARGET)/ping.exe: ping.obj
	mkdir -p ../../exec/$(SPLICE_TARGET)
	$(LD_EXE) $(LDFLAGS_EXE) $(LDFLAGS) $(LDLIBS) $(LDLIBS_SYS) -o ../../exec/$(SPLICE_TARGET)/ping.exe -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) \
		ping.obj pingpongSacDcps.obj pingpongSplDcps.obj $(LIBS)

ping.obj: pingpong.h ../../ping.c pingpongSacDcps.obj pingpongSplDcps.obj
	$(CC) $(CFLAGS)  $(CINCS) -c ../../ping.c
 
../../exec/$(SPLICE_TARGET)/pong.exe: pong.obj
	mkdir -p ../../exec/$(SPLICE_TARGET)
	$(LD_EXE) $(LDFLAGS_EXE) $(LDFLAGS) $(LDLIBS) $(LDLIBS_SYS) -o ../../exec/$(SPLICE_TARGET)/pong.exe -L$(OSPL_HOME)/lib/$(SPLICE_TARGET) \
		pong.obj pingpongSacDcps.obj pingpongSplDcps.obj $(LIBS)

pong.obj: pingpong.h ../../pong.c pingpongSacDcps.obj pingpongSplDcps.obj
	$(CC) $(CFLAGS)  $(CINCS) -c ../../pong.c

pingpongSacDcps.obj: pingpongSacDcps.c pingpongDcps.h pingpongSacDcps.h
	$(CC) $(CFLAGS) $(CINCS) -c pingpongSacDcps.c 

pingpongSplDcps.obj: pingpongSplDcps.c pingpongSplDcps.h pingpongDcps.h
	$(CC) $(CFLAGS) $(CINCS) -c pingpongSplDcps.c

pingpong.h pingpongDcps.h pingpongSacDcps.h pingpongSplDcps.h pingpongSacDcps.c pingpongSplDcps.c: ../../pingpong.idl
	idlpp -S -l c ../../pingpong.idl
