#
# included by bld/$(SPLICE_HOST)/makefile

ifeq (,$(findstring shmt,$(SPLICE_TARGET)))
TARGET_EXEC	:= networking

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += $(LDLIBS_NW) $(LDLIBS_ZLIB) -l$(DDS_USER) -l$(DDS_CONF)
LDLIBS += -l$(DDS_CONFPARSER) -l$(DDS_UTIL) -l$(DDS_KERNEL)
LDLIBS += -l$(DDS_SERIALIZATION) -l$(DDS_DATABASE) -l$(DDS_OS_NET) -l$(DDS_OS)

LDFLAGS += $(LDFLAGS_ZLIB)

# Suppression of macro's (QAC)

CPFLAGS += -macros ../../nw_macros.qac

# Include dirs for this component

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
CINCS += $(CINCS_ZLIB)

-include $(DEPENDENCIES)

else

TARGET_DLIB	:= networking

include $(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS  += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS += $(SHLDFLAGS) $(LDFLAGS_ZLIB)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_OS_NET) -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL)
LDLIBS  += -l$(DDS_USER) -l$(DDS_SERIALIZATION) $(LDLIBS_NW) $(LDLIBS_ZLIB)

# Suppression of macro's (QAC)

CPFLAGS += -macros ../../nw_macros.qac

# Include dirs for this component

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
CINCS += $(CINCS_ZLIB)

-include $(DEPENDENCIES)

endif
