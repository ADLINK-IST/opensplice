
TARGET_DLIB	:= $(DDS_CMCOMMON)

include	$(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS  += $(SHCFLAGS) $(MTCFLAGS)
CINCS   += -I$(OSPL_HOME)/src/database/database/include
CINCS   += -I$(OSPL_HOME)/src/kernel/include
CINCS   += -I$(OSPL_HOME)/src/user/include
CINCS   += -I$(OSPL_HOME)/src/api/dcps/jni/include

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  +=    -l$(DDS_DCPSJNI)

-include $(DEPENDENCIES)
