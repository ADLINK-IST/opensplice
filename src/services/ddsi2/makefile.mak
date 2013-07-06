TARGET_DLIB	:= ddsi2
ODL_MODULES	:= q_osplserModule

include	$(OSPL_HOME)/setup/makefiles/target.mak

##
## FIXME introduce LDLIBS_IN variable
LDLIBS += -l$(DDS_OS)
LDLIBS += -l$(DDS_DATABASE)
LDLIBS += -l$(DDS_KERNEL)
LDLIBS += -l$(DDS_UTIL)
LDLIBS += -l$(DDS_USER)
LDLIBS += -l$(DDS_SERIALIZATION)
LDLIBS += -l$(DDS_CONF)
LDLIBS += -l$(DDS_CONFPARSER)
LDLIBS += $(LDLIBS_NW)

CPPFLAGS += -DOSPL_BUILD_DDSI2 $(OSPL_OS_CPPFLAGS)

CFLAGS  += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)

# Include dirs for this component

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/configuration/config/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
## workaround until v__networkQueue.h has moved from code/ to include/
CINCS += -I$(OSPL_HOME)/src/kernel/code
CINCS += -I$(OSPL_HOME)/src/user/code

#CINCS += -I"$(OPENSSL_HOME)/include"
ifneq "$(WCECOMPAT)" ""
CINCS += "-I$(WCECOMPAT)/include"
endif

LDLIBS  += -l$(DDS_OS_NET) 

-include $(DEPENDENCIES)
