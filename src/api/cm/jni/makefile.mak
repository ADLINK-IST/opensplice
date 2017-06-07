TARGET_DLIB	:= $(DDS_CMJNI)

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_CMJNI
CFLAGS		+= $(SHCFLAGS)
CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/kernel/include
CINCS		+= -I$(OSPL_HOME)/src/user/include
CINCS		+= -I$(OSPL_HOME)/src/api/cm/xml/include
CINCS		+= -I$(OSPL_HOME)/src/database/serialization/include
CINCS		+= $(JAVA_INCLUDE)

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_CMXML)

LDLIBS += -l$(DDS_CORE)

-include $(DEPENDENCIES)
