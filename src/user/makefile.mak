TARGET_DLIB		:= $(DDS_USER)

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_USER
CFLAGS          += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS         += $(SHLDFLAGS)
LDLIBS          += $(SHLDLIBS)

INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/include
INCLUDE		+= -I$(OSPL_HOME)/src/configuration/config/include
INCLUDE		+= -I$(OSPL_HOME)/src/configuration/parser/include

LDLIBS		+= -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) -l$(DDS_CONF) -l$(DDS_CONFPARSER)

-include $(DEPENDENCIES)
