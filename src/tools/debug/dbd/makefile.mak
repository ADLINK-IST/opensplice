#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= dbd

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS := -l$(DDS_OS) $(LDLIBS)
LDLIBS := -l$(DDS_KERNEL) -l$(DDS_DATABASE) -l$(DDS_UTIL) -l$(DDS_SERIALIZATION) $(LDLIBS)
LDLIBS := -l$(DDS_USER) -l$(DDS_CONF) -l$(DDS_CONFPARSER) $(LDLIBS)

CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/code
CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/user/code
CINCS += -I$(OSPL_HOME)/src/configuration/config/include
CINCS += -I$(OSPL_HOME)/src/utilities/include

-include $(DEPENDENCIES)
