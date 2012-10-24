#
# included by bld/$(SPLICE_HOST)/makefile

ifeq (,$(findstring shmt,$(SPLICE_TARGET)))
TARGET_EXEC	:= ddsi2
ODL_MODULES	:= nn_osplserModule

include	$(OSPL_HOME)/setup/makefiles/target.mak

##
## FIXME introduce LDLIBS_IN variable
LDLIBS += -l$(DDS_OS)
LDLIBS += -l$(DDS_DATABASE)
LDLIBS += -l$(DDS_KERNEL)
LDLIBS += -l$(DDS_USER)
LDLIBS += -l$(DDS_SERIALIZATION)
LDLIBS += -l$(DDS_CONF)
LDLIBS += -l$(DDS_CONFPARSER)
LDLIBS += -l$(DDS_UTIL)
LDLIBS += $(LDLIBS_NW)

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

LDLIBS  += -l$(DDS_OS_NET) 

-include $(DEPENDENCIES)

else

TARGET_DLIB	:= ddsi2

include $(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS  += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) 
LDLIBS  += -l$(DDS_USER) -l$(DDS_SERIALIZATION) $(LDLIBS_NW)

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
