#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= mmstat

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += -l$(DDS_SERIALIZATION) -l$(DDS_CONFPARSER)  -lwsock32
LDLIBS += -l$(DDS_USER) -l$(DDS_CONF) -l$(DDS_KERNEL) 
LDLIBS += -l$(DDS_DATABASE) -l$(DDS_UTIL) -l$(DDS_OS) 

CINCS += -I$(OSPL_HOME)/src/abstraction/os/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/code
CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/user/code
CINCS += -I$(OSPL_HOME)/src/configuration/config/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
CINCS   += -I$(OSPL_HOME)/src/tools/mmstat/common/include

-include $(DEPENDENCIES)
