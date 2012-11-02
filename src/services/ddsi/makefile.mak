#
# included by bld/$(SPLICE_HOST)/makefile

ifeq (,$(findstring shmt,$(SPLICE_TARGET)))
TARGET_EXEC	:= ddsi

include	$(OSPL_HOME)/setup/makefiles/target.mak

##
## FIXME introduce LDLIBS_IN variable
LDLIBS += $(LDLIBS_NW)
LDLIBS += -l$(DDS_USER)
LDLIBS += -l$(DDS_CONF)
LDLIBS += -l$(DDS_CONFPARSER)
LDLIBS += -l$(DDS_UTIL)
LDLIBS += -l$(DDS_KERNEL)
LDLIBS += -l$(DDS_SERIALIZATION)
LDLIBS += -l$(DDS_DATABASE)
LDLIBS += -l$(DDS_OS_NET)
LDLIBS += -l$(DDS_OS)

# Suppression of macro's (QAC)

CPFLAGS += -macros ../../nw_macros.qac
# Include dirs for this component

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
## workaround until v__networkQueue.h has moved from code/ to include/
CINCS += -I$(OSPL_HOME)/src/kernel/code
CINCS += -I$(OSPL_HOME)/src/user/code

-include $(DEPENDENCIES)

else

TARGET_DLIB	:= ddsi

include $(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS  += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += $(LDLIBS_NW) -l$(DDS_USER) -l$(DDS_KERNEL) -l$(DDS_SERIALIZATION)
LDLIBS  += -l$(DDS_DATABASE) -l$(DDS_OS)

# Suppression of macro's (QAC)

CPFLAGS += -macros ../../nw_macros.qac

# Include dirs for this component

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
## workaround until v__networkQueue.h has moved from code/ to include/
CINCS += -I$(OSPL_HOME)/src/kernel/code
CINCS += -I$(OSPL_HOME)/src/user/code

-include $(DEPENDENCIES)

endif
