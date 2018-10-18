# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= topmon

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += $(CORE_LIBS) -l$(DDS_DCPSSAC) -l$(DDS_CORE)

CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/include
CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/bld/$(SPLICE_TARGET)
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/configuration/config/include
CINCS += -I$(OSPL_HOME)/src/utilities/include

-include $(DEPENDENCIES)
