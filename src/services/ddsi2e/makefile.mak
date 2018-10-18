TARGET_DLIB	:= ddsi2e
ODL_MODULES := q_osplserModule

C_FILES := $(notdir $(filter-out %_template.c, $(wildcard ../../code/*.c ../../core/*.c)))
include	$(OSPL_HOME)/setup/makefiles/target.mak
-include $(OSPL_OUTER_HOME)/setup/makefiles/commercial.mak

DDSI2E_CONFIG = -DDDSI2E_OR_NOT2E -DDDSI_INCLUDE_NETWORK_PARTITIONS -DDDSI_INCLUDE_NETWORK_CHANNELS -DDDSI_INCLUDE_BANDWIDTH_LIMITING -DDDSI_INCLUDE_SSM
ifeq "$(INCLUDE_SECURITY)" "yes"
DDSI2E_CONFIG += -DDDSI_INCLUDE_ENCRYPTION -DDDSI_INCLUDE_SSL
endif

CPPFLAGS += $(DDSI2E_CONFIG)
CPPFLAGS += -DOSPL_BUILD_DDSI2 -DMODEL_q_osplserModule_IMPLEMENTATION

CFLAGS  += $(SHCFLAGS)
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)

CFLAGS += $(CFLAGS_XSTRICT)
CFLAGS += $(CFLAGS_W_ERROR)
#ddsi_ssl$(OBJ_POSTFIX) ddsi_tcp$(OBJ_POSTFIX) q_sockwaitset$(OBJ_POSTFIX): CFLAGS += $(CFLAGS_W_NOERROR)

ifneq (,$(findstring solaris10_gcc,$(SPLICE_TARGET)))
LDFLAGS += -mimpure-text
endif

VPATH = 
vpath %.c
vpath %.c ../../code ../../core

CPPFLAGS += $(CPPFLAGS_T_RLM)
CFLAGS   += $(CFLAGS_T_RLM)

##
## FIXME introduce LDLIBS_IN variable
ifneq "$(INCLUDE_SECURITY)" ""
LDLIBS += $(LDLIBS_NW_SEC)
endif

LDLIBS += -l$(DDS_CORE)
LDLIBS += $(LDLIBS_NW)
LDLIBS += $(LDLIBS_ZLIB) 

# Include dirs for this component

CINCS += -I../../code
CINCS += -I../../core

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/configuration/config/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
## workaround until v__networkQueue.h has moved from code/ to include/
CINCS += -I$(OSPL_HOME)/src/kernel/code
CINCS += -I$(OSPL_HOME)/src/user/code
CINCS += -I$(OSPL_HOME)/src/configuration/config/include
CINCS += -I"$(OPENSSL_TARGET_HOME)/include"
CINCS += $(CINCS_NW_SEC)
ifneq "$(WCECOMPAT)" ""
CINCS += "-I$(WCECOMPAT)/include"
endif
CINCS += $(CINCS_ZLIB)

LDFLAGS += $(LDFLAGS_ZLIB)

# automagic generation/updating of osplconf input
METACONFIG := splice_metaconfig_$(firstword $(subst ., , $(patsubst V%, %, $(PACKAGE_VERSION)))).1.xml

.PHONY: osplconf osplconf-update
osplconf: $(METACONFIG)

.PHONY: ddsi2e-config
ddsi2e-config:
	@echo $(DDSI2E_CONFIG)

# don't want a cyclical dependency
osplconf-update: $(METACONFIG)
	if [ $(METACONFIG) -nt $(OSPL_HOME)/src/tools/cm/config/code/$(METACONFIG) ] ; then \
		cp -p $< $(OSPL_HOME)/src/tools/cm/config/code/$(METACONFIG) ; \
		$(MAKE) -C $(OSPL_HOME)/src/tools/cm/config ; \
	fi

$(METACONFIG): ../../extract-configurator-xml.awk ../../core/q_config.c $(OSPL_HOME)/src/services/ddsi2/code/q_config.c $(OSPL_HOME)/src/tools/cm/config/code/$(METACONFIG) ../../merge-metaconfig
	gawk -vversion=COMMERCIAL -f ../../extract-configurator-xml.awk ../../core/q_config.c >config-ddsi2e.xml
	$(SHELL) ../../merge-metaconfig $(OSPL_HOME)/src/tools/cm/config/code/$(METACONFIG) config-ddsi2e.xml >$@.tmp
	gawk -vversion=COMMUNITY -f ../../extract-configurator-xml.awk $(OSPL_HOME)/src/services/ddsi2/code/q_config.c >config-ddsi2.xml
	$(SHELL) ../../merge-metaconfig $@.tmp config-ddsi2.xml >$@
	rm -f $@.tmp

-include $(DEPENDENCIES)
