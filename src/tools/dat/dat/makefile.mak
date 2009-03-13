TARGET_EXEC		:= dat

include		$(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS          += -l$(DDS_OS)
LDLIBS		+= -l$(DDS_DATABASE)
LDLIBS		+= -laapi

INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/tools/dat/aapi/include

-include $(DEPENDENCIES)
