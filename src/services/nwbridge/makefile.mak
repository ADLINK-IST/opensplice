TARGET_DLIB := nwbridge
ODL_MODULES := NetworkingBridge

include $(OSPL_HOME)/setup/makefiles/target.mak

ifdef ELINOS42
CFLAGS	+= -std=c99
endif

LDFLAGS  += $(LDFLAGS_T_RLM) $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_CORE) $(LDLIBS_T_RLM)

CPPFLAGS += $(CPPFLAGS_T_RLM)
CFLAGS   += $(CFLAGS_T_RLM)

CPPFLAGS += -DOSPL_BUILD_NWBRIDGE_SERVICE

CFLAGS   += $(SHCFLAGS)
CFLAGS += $(CFLAGS_XSTRICT)

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
CINCS += -I$(OSPL_HOME)/src/configuration/parser/include
CINCS += -I$(OSPL_HOME)/src/configuration/config/include
CINCS += -I$(OSPL_HOME)/src/services/nwbridge/bld/$(SPLICE_TARGET)

ifneq (,$(findstring int5,$(SPLICE_TARGET)))
CFLAGS += -DCOMPILE_ENTRYPOINT_AS_MAIN
endif

# automagic generation/updating of osplconf input
METACONFIG := splice_metaconfig_$(firstword $(subst ., , $(patsubst V%, %, $(PACKAGE_VERSION)))).1.xml

.PHONY: osplconf osplconf-update
osplconf: $(METACONFIG)

# don't want a cyclical dependency
osplconf-update: $(METACONFIG)
	if [ $(METACONFIG) -nt $(OSPL_HOME)/src/tools/cm/config/code/$(METACONFIG) ] ; then \
		cp -p $< $(OSPL_HOME)/src/tools/cm/config/code/$(METACONFIG) ; \
		$(MAKE) -C $(OSPL_HOME)/src/tools/cm/config ; \
	fi

$(METACONFIG): ../../../ddsi2e/extract-configurator-xml.awk ../../code/nb_configuration.c $(OSPL_HOME)/src/tools/cm/config/code/$(METACONFIG) ../../../ddsi2e/merge-metaconfig
	gawk -vversion=COMMERCIAL -f ../../../ddsi2e/extract-configurator-xml.awk ../../code/nb_configuration.c >config-nwbridge.xml
	$(SHELL) ../../../ddsi2e/merge-metaconfig $(OSPL_HOME)/src/tools/cm/config/code/$(METACONFIG) config-nwbridge.xml >$@

-include $(DEPENDENCIES)
