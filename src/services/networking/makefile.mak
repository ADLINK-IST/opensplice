#
# included by bld/$(SPLICE_HOST)/makefile

ifeq (,$(findstring shmt,$(SPLICE_TARGET)))
TARGET_EXEC	:= networking

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += $(LDLIBS_NW) -l$(DDS_USER) -l$(DDS_CONF) -l$(DDS_CONFPARSER) 
LDLIBS += -l$(DDS_UTIL) -l$(DDS_KERNEL) -l$(DDS_SERIALIZATION) 
LDLIBS += -l$(DDS_DATABASE) -l$(DDS_OS_NET) -l$(DDS_OS)

# Suppression of macro's (QAC)

CPFLAGS += -macros ../../nw_macros.qac

# Include dirs for this component

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include

-include $(DEPENDENCIES)

else

TARGET_DLIB	:= networking

include $(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS  += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_OS_NET) -l$(DDS_OS) -l$(DDS_DATABASE) 
LDLIBS  += -l$(DDS_KERNEL) -l$(DDS_USER) -l$(DDS_SERIALIZATION) $(LDLIBS_NW)

# Suppression of macro's (QAC)

CPFLAGS += -macros ../../nw_macros.qac

# Include dirs for this component

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include


-include $(DEPENDENCIES)

endif
