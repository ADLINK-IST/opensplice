
TARGET := libcmc.so
TARGET_LINK_DIR := $(SPLICE_LIBRARY_PATH)

include $(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS  += $(SHCFLAGS) $(MTCFLAGS)
CINCS	  += -I$(OSPL_HOME)/src/database/database/include
CINCS	  += -I$(OSPL_HOME)/src/kernel/include
CINCS	  += -I$(OSPL_HOME)/src/user/include
CINCS	  += -I$(OSPL_HOME)/src/api/cm/common/include

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_CORE) -l$(DDS_CMCOMMON)

$(TARGET):	$(OBJECTS)

-include $(DEPENDENCIES)
