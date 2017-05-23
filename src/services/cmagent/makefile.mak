TARGET_DLIB := $(DDS_CMAGENT)

IDL_DIR = $(OSPL_HOME)/etc/idl
IDL_FILES = vortex_agent.idl vortex_metrics.idl

include $(OSPL_HOME)/setup/makefiles/test_idl_c.mak

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS += -DOSPL_BUILD_CMAGENT_SERVICE

ifdef ELINOS42
CFLAGS += -std=c99
endif

CFLAGS += $(SHCFLAGS)
CFLAGS += $(CFLAGS_XSTRICT)

LDFLAGS += $(SHLDFLAGS)
LDLIBS += $(SHLDLIBS)
LDLIBS += -l$(DDS_CORE) -l$(DDS_DCPSSAC)

CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/include
CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/bld/$(SPLICE_TARGET)

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
CINCS += -I$(OSPL_HOME)/src/configuration/parser/include
CINCS += -I$(OSPL_HOME)/src/configuration/config/include
#CINCS += -I$(OSPL_HOME)/src/services/cmagent/bld/$(SPLICE_TARGET)

ifneq (,$(findstring int5,$(SPLICE_TARGET)))
CFLAGS += -DCOMPILE_ENTRYPOINT_AS_MAIN
endif

.PHONY: agentconf

agentconf:
	@../../extract.py $(CINCS) ../../code/cma_configuration.c

-include $(DEPENDENCIES)
