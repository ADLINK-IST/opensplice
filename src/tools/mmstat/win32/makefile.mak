#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= mmstat

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += -l$(DDS_DATABASE) -l$(DDS_USER) -l$(DDS_UTIL) 
LDLIBS += -l$(DDS_KERNEL) -l$(DDS_CONF) -l$(DDS_SERIALIZATION)
LDLIBS += -l$(DDS_CONFPARSER) -l$(DDS_OS) -lwsock32


CINCS	+= -I$(OSPL_HOME)/src/abstraction/os/include
CINCS	+= -I$(OSPL_HOME)/src/database/database/include
CINCS	+= -I$(OSPL_HOME)/src/kernel/include
CINCS	+= -I$(OSPL_HOME)/src/database/database/code
CINCS	+= -I$(OSPL_HOME)/src/user/include
CINCS	+= -I$(OSPL_HOME)/src/user/code
CINCS += -I$(OSPL_HOME)/src/utilities/include

-include $(DEPENDENCIES)
