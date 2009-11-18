TARGET_DLIB	:= $(DDS_CMXML)

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_CMXML
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS    += -I$(OSPL_HOME)/src/database/database/include
CINCS    += -I$(OSPL_HOME)/src/kernel/include
CINCS    += -I$(OSPL_HOME)/src/kernel/code
CINCS    += -I$(OSPL_HOME)/src/user/include
CINCS    += -I$(OSPL_HOME)/src/user/code
CINCS    += -I$(OSPL_HOME)/src/database/serialization/include
CINCS    += $(JAVA_INCLUDE)

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) -l$(DDS_USER) 
LDLIBS   += -l$(DDS_SERIALIZATION)

-include $(DEPENDENCIES)
