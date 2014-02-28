TARGET_DLIB	:= $(DDS_DCPSJNI)

include	$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_DCPSJNI
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += -l$(DDS_CORE) $(SHLDLIBS)

CINCS	+= -I$(OSPL_HOME)/src/abstraction/include
CINCS	+= -I$(OSPL_HOME)/src/database/database/include
CINCS	+= -I$(OSPL_HOME)/src/user/include
CINCS	+= -I$(OSPL_HOME)/src/kernel/include
CINCS	+= -I$(OSPL_HOME)/src/database/serialization/include
CINCS	+= $(JAVA_INCLUDE)


-include $(DEPENDENCIES)
