TARGET_DLIB	:= $(DDS_DCPSJNI)

include	$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_DCPSJNI
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)

CINCS	+= -I$(OSPL_HOME)/src/abstraction/include
CINCS	+= -I$(OSPL_HOME)/src/database/database/include
CINCS	+= -I$(OSPL_HOME)/src/user/include
CINCS	+= -I$(OSPL_HOME)/src/kernel/include
CINCS	+= -I$(OSPL_HOME)/src/database/serialization/include
CINCS	+= $(JAVA_INCLUDE)

LDLIBS += -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) 
LDLIBS += -l$(DDS_USER) -l$(DDS_SERIALIZATION)

-include $(DEPENDENCIES)
