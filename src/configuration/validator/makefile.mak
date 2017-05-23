#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_EXEC	:= chkconf

include $(OSPL_HOME)/setup/makefiles/target.mak

#CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CFLAGS   += $(MTCFLAGS)
CINCS    += -I../../../common/include
CINCS    += -I$(OSPL_HOME)/src/utilities/include
CINCS    += -I$(OSPL_HOME)/src/database/serialization/include
CINCS    += -I$(OSPL_HOME)/src/database/database/include
CINCS    += -I$(OSPL_HOME)/src/kernel/include
CINCS    += -I$(OSPL_HOME)/src/user/include
CINCS    += -I$(OSPL_HOME)/src/configuration/config/include
CINCS    += -I$(OSPL_HOME)/src/configuration/parser/include
CINCS    += -I$(OSPL_HOME)/src/api/dcps/common/include

#LDFLAGS   += $(SHLDFLAGS)
LDLIBS    += $(SHLDLIBS)
LDLIBS    += -l$(DDS_CORE)

-include $(DEPENDENCIES)
