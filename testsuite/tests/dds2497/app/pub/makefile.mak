# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC     := dds2497_pub
TARGET_LINK_DIR := ../../../exec/$(SPLICE_TARGET)

include $(OSPL_OUTER_HOME)/setup/makefiles/test_target.mak

CINCS += -I$(OSPL_OUTER_HOME)/testsuite/rbt/services/networking/dds2497/idl/bld/$(SPLICE_TARGET)
CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/include
CINCS += -I$(OSPL_HOME)/src/api/cm/xml/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_OUTER_HOME)/testsuite/rbt/sac/common/include

LDLIBS +=  -l$(DDS_DCPSSAC)  -l$(DDS_CMXML)
LDLIBS += -l$(DDS_CORE)   
LDLIBS +=    -ldds2497

ifeq ($(SPECIAL),purify)
CFLAGS += -D_PURIFY
endif

-include $(DEPENDENCIES)
