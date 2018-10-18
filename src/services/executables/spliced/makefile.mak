TARGET_EXEC	:= spliced

include $(OSPL_HOME)/setup/makefiles/target.mak

CINCS  += -I$(OSPL_HOME)/src/services/spliced/code
CINCS  += -I$(OSPL_HOME)/src/database/database/include
CINCS  += -I$(OSPL_HOME)/src/database/serialization/include
CINCS  += -I$(OSPL_HOME)/src/kernel/include
CINCS  += -I$(OSPL_HOME)/src/user/include
CINCS  += -I$(OSPL_HOME)/src/utilities/include

LDLIBS := -l$(DDS_SPLICED) -l$(DDS_CORE) $(LDLIBS)

ifdef STATIC_LIB_ONLY
spliced:$(OSPL_HOME)/lib/$(SPLICE_TARGET)/libspliced.a
endif
