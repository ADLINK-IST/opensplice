#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= dbd

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS :=  $(LDLIBS)
LDLIBS :=   -l$(DDS_CORE)  $(LDLIBS)
LDLIBS :=    $(LDLIBS)

CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/code
CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/user/code
CINCS += -I$(OSPL_HOME)/src/configuration/config/include
CINCS += -I$(OSPL_HOME)/src/utilities/include

-include $(DEPENDENCIES)
