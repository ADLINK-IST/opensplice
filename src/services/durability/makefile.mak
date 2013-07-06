TARGET_DLIB := durability
ODL_MODULES := durabilityModule2

ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
ODL_MODULES	+= kernelModule
endif

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS += -DOSPL_BUILD_DURABILITY $(DURABILITY_PLUGIN_CPPFLAGS)

CFLAGS += $(SHCFLAGS) $(MTCFLAGS)
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
LDLIBS  += -l$(DDS_OS) -l$(DDS_USER) -l$(DDS_KERNEL)
LDLIBS  += -l$(DDS_SERIALIZATION) -l$(DDS_DATABASE) -l$(DDS_UTIL)
LDLIBS  += $(DURABILITY_PLUGIN_LIBS)

ifneq (,$(findstring int5,$(SPLICE_TARGET)))
CFLAGS += -DCOMPILE_ENTRYPOINT_AS_MAIN
endif

ifneq (,$(DDS_DURABILITY_EXT_MAKE))
-include $(DDS_DURABILITY_EXT_MAKE)
endif

-include $(DEPENDENCIES)
