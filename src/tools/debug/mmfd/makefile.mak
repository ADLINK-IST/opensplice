#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= mmfd

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += -l$(DDS_USER) -l$(DDS_CONF) -l$(DDS_CONFPARSER)
LDLIBS += -l$(DDS_KERNEL) -l$(DDS_SERIALIZATION)
LDLIBS += -l$(DDS_DATABASE) -l$(DDS_UTIL) -l$(DDS_OS)


CINCS	+= -I$(OSPL_HOME)/src/database/database/include
CINCS	+= -I$(OSPL_HOME)/src/kernel/include
CINCS	+= -I$(OSPL_HOME)/src/database/database/code
CINCS	+= -I$(OSPL_HOME)/src/user/include
CINCS	+= -I$(OSPL_HOME)/src/user/code
CINCS   += -I$(OSPL_HOME)/src/utilities/include
CINCS   += -I$(OSPL_HOME)/src/services/durability/include
CINCS   += -I$(OSPL_HOME)/src/services/durability/code
CINCS   += -I$(OSPL_HOME)/src/services/durability/bld/$(SPLICE_TARGET)

-include $(DEPENDENCIES)
