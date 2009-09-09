#include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak

all link: .pingpong

.pingpong: ../../ping.java ../../pinger.java ../../stats.java ../../time.java \
	   ../../pong.java ../../ponger.java ../../pingpong.idl
	sh ../../build_example
	>.pingpong
