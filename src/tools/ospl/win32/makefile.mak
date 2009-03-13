# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= ospl

include		$(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS		+= -l$(DDS_OS) -l$(DDS_CONF) -l$(DDS_CONFPARSER)

INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/configuration/config/include
INCLUDE		+= -I$(OSPL_HOME)/src/configuration/parser/include

-include $(DEPENDENCIES)
