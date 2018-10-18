TARGET_DLIB := durability
ODL_MODULES := durabilityModule2
IDL_DIR   = ../../code:$(OSPL_HOME)/etc/idl
IDL_FILES = dds_builtinTopics.idl dds_dcps_builtintopics.idl client_durability.idl

# Get the default idlpp setting
include $(OSPL_HOME)/setup/makefiles/test_idl_c.mak

# Use the following settings to parse the client-durability idl
IDL_C        =
IDL_H        = $(IDL_TYPE_FILES) $(IDL_LOAD_FILES)
IDLPPFLAGS  += -I$(OSPL_HOME)/etc/idl -N

# Build durability library
include $(OSPL_HOME)/setup/makefiles/target.mak
-include $(OSPL_OUTER_HOME)/setup/makefiles/commercial.mak

CPPFLAGS += -DOSPL_BUILD_DURABILITY -DMODEL_durabilityModule2_IMPLEMENTATION $(DURABILITY_PLUGIN_CPPFLAGS)

CFLAGS += $(SHCFLAGS)
CFLAGS += $(CFLAGS_XSTRICT) $(CFLAGS_W_ERROR)

CINCS  += -I$(OSPL_HOME)/src/utilities/include
CINCS  += -I$(OSPL_HOME)/src/database/serialization/include
CINCS  += -I$(OSPL_HOME)/src/database/database/include
CINCS  += -I$(OSPL_HOME)/src/kernel/include
CINCS  += -I$(OSPL_HOME)/src/user/include
CINCS  += -I$(OSPL_HOME)/src/configuration/config/include
CINCS  += -I$(OSPL_HOME)/src/services/durability/bld/$(SPLICE_TARGET)
CINCS  += $(DDS_DURABILITY_TMPL_INC) $(DURABILITY_PLUGIN_CINCS)

SPPODL_FLAGS += -I$(OSPL_HOME)/src/kernel/code -I$(OSPL_HOME)/src/abstraction/os/include

vpath %.odl $(OSPL_HOME)/src/kernel/code
durabilityModule2.h: v_kernel.odl

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_CORE)
LDLIBS  += $(DURABILITY_PLUGIN_LIBS)

include ../../makefile.ext

-include $(DEPENDENCIES)
