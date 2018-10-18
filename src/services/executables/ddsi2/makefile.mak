TARGET_EXEC	:= ddsi2

include	$(OSPL_HOME)/setup/makefiles/target.mak

CINCS += -I$(OSPL_HOME)/src/services/ddsi2/code
CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
## workaround until v__networkQueue.h has moved from code/ to include/
CINCS += -I$(OSPL_HOME)/src/kernel/code
CINCS += -I$(OSPL_HOME)/src/user/code

LDLIBS := -l$(DDS_DDSI2) -l$(DDS_CORE) $(LDLIBS_NW) $(LDLIBS)

ifeq "$(INCLUDE_SECURITY)" "yes"
LDLIBS += $(LDLIBS_NW_SEC)
endif

