# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC		:= qosprovider_c
TARGET_LINK_DIR	:= ../../exec/$(SPLICE_TARGET)

include		$(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += $(LDLIBS_OS) -l$(DDS_DCPSSAC) -l$(DDS_CORE)

CINCS += -I$(OSPL_HOME)/src/api/dcps/common/include
CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/include
CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/code
CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/bld/$(SPLICE_TARGET)
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
