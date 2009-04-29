#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC		:= mmstat

include		$(OSPL_HOME)/setup/makefiles/target.mak

LDFLAGS     += $(LDFLAGS_FLEX)

LDLIBS		+= -l$(DDS_DATABASE) -l$(DDS_USER) -l$(DDS_UTIL) 
LDLIBS		+= -l$(DDS_KERNEL) -l$(DDS_CONF) -l$(DDS_SERIALIZATION)
LDLIBS		+= -l$(DDS_CONFPARSER) -l$(DDS_OS) $(LDLIBS_FLEX)


INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/database/code
INCLUDE		+= -I$(OSPL_HOME)/src/user/include
INCLUDE		+= -I$(OSPL_HOME)/src/user/code
INCLUDE     += -I$(OSPL_HOME)/src/utilities/include
INCLUDE     += -I$(OSPL_OUTER_HOME)/src/utilities/license/include

-include $(DEPENDENCIES)
