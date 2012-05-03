#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= odlpp

include $(OSPL_HOME)/setup/makefiles/target.mak

ifneq (,$(ALT_CFLAGS_OPT))
   CFLAGS_OPT=$(ALT_CFLAGS_OPT)
endif

LDFLAGS += -L$(OSPL_HOME)/lib/$(SPLICE_HOST)
LDLIBS  += -l$(DDS_DATABASE) -l$(DDS_OS)
LDLIBS  :=$(filter-out -lefence, $(LDLIBS))

CINCS += -I../../../database/include
CINCS += -I../../../abstraction/os/include

-include $(DEPENDENCIES)
