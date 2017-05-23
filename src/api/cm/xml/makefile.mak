TARGET_DLIB	:= $(DDS_CMXML)

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_CMXML
CFLAGS   += $(SHCFLAGS)
CINCS    += -I$(OSPL_HOME)/src/database/database/include
CINCS    += -I$(OSPL_HOME)/src/kernel/include
CINCS    += -I$(OSPL_HOME)/src/kernel/code
CINCS    += -I$(OSPL_HOME)/src/user/include
CINCS    += -I$(OSPL_HOME)/src/user/code
CINCS    += -I$(OSPL_HOME)/src/database/serialization/include
CINCS    += -I$(OSPL_HOME)/src/configuration/config/include
CINCS    += -I$(OSPL_HOME)/src/utilities/include
CINCS    += $(JAVA_INCLUDE)
CINCS    += $(DDS_RRSTORAGE_INCS)
CINCS    += $(DDS_CMX_STORAGE_TMPL_INC)

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += $(DDS_RRSTORAGE_LIBS)

LDLIBS += -l$(DDS_CORE)

-include $(DEPENDENCIES)
