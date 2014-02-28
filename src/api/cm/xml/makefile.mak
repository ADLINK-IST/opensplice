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
CINCS    += $(DDS_RRSTORAGE_INCS)
CINCS    += $(DDS_CMX_STORAGE_TMPL_INC)

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += $(DDS_RRSTORAGE_LIBS)

#if we are building for the host then use ddshts lib
#as there is no ddskernel
ifneq ($(SPLICE_TARGET),$(SPLICE_REAL_TARGET))
LDLIBS += -l$(DDS_HTS)
else
LDLIBS += -l$(DDS_CORE)
endif

-include $(DEPENDENCIES)
