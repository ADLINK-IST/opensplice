#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= mmstat

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS +=    -lwsock32
LDLIBS +=  -l$(DDS_CORE)  

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
