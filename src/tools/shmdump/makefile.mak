TARGET_EXEC	:= shmdump

include		$(OSPL_HOME)/setup/makefiles/target.mak


INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
LDLIBS += -l$(DDS_CORE)

-include $(DEPENDENCIES)
