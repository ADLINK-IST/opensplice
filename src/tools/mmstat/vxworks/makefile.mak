#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC		:= mmstat

include		$(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS		+= -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) -l$(DDS_USER) -l$(DDS_UTIL)

INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/database/code
INCLUDE		+= -I$(OSPL_HOME)/src/user/include
INCLUDE		+= -I$(OSPL_HOME)/src/user/code
INCLUDE     += -I$(OSPL_HOME)/src/utilities/include

-include $(DEPENDENCIES)
