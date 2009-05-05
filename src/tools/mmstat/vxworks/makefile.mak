#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= mmstat

include	$(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) 
LDLIBS +=-l$(DDS_USER) -l$(DDS_UTIL)


CINCS	+= -I$(OSPL_HOME)/src/database/database/include
CINCS	+= -I$(OSPL_HOME)/src/kernel/include
CINCS	+= -I$(OSPL_HOME)/src/database/database/code
CINCS	+= -I$(OSPL_HOME)/src/user/include
CINCS	+= -I$(OSPL_HOME)/src/user/code
CINCS += -I$(OSPL_HOME)/src/utilities/include

-include $(DEPENDENCIES)
