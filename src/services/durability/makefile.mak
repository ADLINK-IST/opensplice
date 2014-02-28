TARGET_DLIB := durability
ODL_MODULES := durabilityModule2

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS += -DOSPL_BUILD_DURABILITY -DMODEL_durabilityModule2_IMPLEMENTATION $(DURABILITY_PLUGIN_CPPFLAGS)

CFLAGS += $(SHCFLAGS) $(MTCFLAGS)
CINCS  += -I$(OSPL_HOME)/src/utilities/include
CINCS  += -I$(OSPL_HOME)/src/database/serialization/include
CINCS  += -I$(OSPL_HOME)/src/database/database/include
CINCS  += -I$(OSPL_HOME)/src/kernel/include
CINCS  += -I$(OSPL_HOME)/src/user/include
CINCS  += -I$(OSPL_HOME)/src/configuration/config/include
CINCS  += -I$(OSPL_HOME)/src/services/durability/bld/$(SPLICE_TARGET)
CINCS  += $(DDS_DURABILITY_TMPL_INC) $(DURABILITY_PLUGIN_CINCS)

SPPODL_FLAGS += -I$(OSPL_HOME)/src/kernel/code

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  +=   -l$(DDS_CORE)
LDLIBS  += $(DURABILITY_PLUGIN_LIBS)

ifneq (,$(findstring int5,$(SPLICE_TARGET)))
CFLAGS += -DCOMPILE_ENTRYPOINT_AS_MAIN
endif

ifneq (,$(DDS_DURABILITY_EXT_MAKE))
-include $(DDS_DURABILITY_EXT_MAKE)
endif

-include $(DEPENDENCIES)
