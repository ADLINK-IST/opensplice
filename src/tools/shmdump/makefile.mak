TARGET_EXEC	:= shmdump

include		$(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS          += -l$(DDS_OS)

INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include

-include $(DEPENDENCIES)
