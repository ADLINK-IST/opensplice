TARGET_EXEC	:= dat

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += -laapi

CINCS  += -I$(OSPL_HOME)/src/database/database/include
CINCS  += -I$(OSPL_HOME)/tools/dat/aapi/include

-include $(DEPENDENCIES)
