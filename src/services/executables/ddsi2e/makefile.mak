TARGET_EXEC	:= ddsi2e

include	$(OSPL_HOME)/setup/makefiles/target.mak

CINCS += -I$(OSPL_HOME)/src/services/ddsi2e/code
CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
## workaround until v__networkQueue.h has moved from code/ to include/
CINCS += -I$(OSPL_HOME)/src/kernel/code
CINCS += -I$(OSPL_HOME)/src/user/code

CPPFLAGS += -DDDSI2E_OR_NOT2E

LDLIBS := -l$(DDS_DDSI2E) -l$(DDS_CORE) $(LDLIBS_NW) $(LDLIBS) $(LDLIBS_NW_SEC)
