#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= mmfd

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS +=  -l$(DDS_CORE) 


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
