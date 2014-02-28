# included by bld/$(SPLICE_HOST)/makefile

TARGET_DLIB	:= dds2497
TARGET_LINK_DIR := $(OSPL_OUTER_HOME)/testsuite/lib/$(SPLICE_TARGET)

TOPIC_IDL	:= net2497.idl

include $(OSPL_HOME)/setup/makefiles/test_idl_c.mak
include $(OSPL_OUTER_HOME)/setup/makefiles/test_target.mak

# Finetune idlpp compiler settings when running in Windows.
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
IDLPPFLAGS += -P COMMON_SAC_API,common_sac_if.h
CPPFLAGS   += -DOSPL_BUILD_CHECK_DCPS -DCOMMON_SAC_BUILD
endif

CFLAGS += $(SHCFLAGS) $(MTCFLAGS)

CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_OUTER_HOME)/testsuite/rbt/sac/common/include

LDFLAGS += $(SHLDFLAGS)
LDLIBS += $(SHLDLIBS) -l$(DDS_DCPSSAC) 

ifeq ($(SPECIAL),purify)
CFLAGS += -D_PURIFY
endif

-include $(DEPENDENCIES)
