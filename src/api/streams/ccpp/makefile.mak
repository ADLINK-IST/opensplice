.NOTPARALLEL:

TARGET_DLIB	:= $(DDS_STREAMSCCPP)

IDL_DIR         := $(OSPL_HOME)/src/api/streams/idl
STREAMS_API_IDL := streams_dcps.idl

DECL_PREFIX := OS_STREAMS_API
DECL_INCLUDE := ccpp_streams_if.h

include $(OSPL_HOME)/setup/makefiles/orbdeps.mak

IDL_FILES     	:= $(STREAMS_API_IDL)
IDL_INC_FLAGS 	:= -I"$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_HOME)/etc/idl)" -I"$(shell ospl_normalizePath $(IDL_DIR))"
vpath %.idl	$(IDL_DIR)

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile
# IDL_O will be linked into the final target
IDL_O = $(STREAMS_API_OBJ)
IDL_C = $(STREAMS_API_SRC)
IDL_H = $(STREAMS_API_HDR)

include $(OSPL_HOME)/setup/makefiles/target.mak

ORB_DIR     := $(IDL_DIR)/$(SPLICE_TARGET)/$(SPLICE_ORB)
ORB_TARGETS := $(STREAMS_API_OBJ)

CPPFLAGS	+= -DOSPL_BUILD_STREAMSCCPP $(ORB_CPP_FLAGS)

CXXFLAGS += $(SHCFLAGS)

# If specific linker options for CXX are set, use those to overrule the SHLDFLAGS
ifneq (,$(SHLDCXXFLAGS))
SHLDFLAGS = $(SHLDCXXFLAGS)
endif

# Fine tune the Linker flags.
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS) $(LDLIBS_CXX) $(ORB_LDLIBS)
LDLIBS	+= -l$(DDS_DCPSCCPP) -l$(DDS_CORE)

CXXINCS += $(ORB_INCLUDE)
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/c++/common/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/c++/ccpp/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/c++/ccpp/include/orb/$(SPLICE_ORB)
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/c++/ccpp/bld/$(SPLICE_TARGET)
CXXINCS += -I$(OSPL_HOME)/src/abstraction/include
CXXINCS += -I$(OSPL_HOME)/src/database/database/include
CXXINCS += -I$(OSPL_HOME)/src/kernel/include
CXXINCS += -I$(OSPL_HOME)/src/user/include
CXXINCS += -I$(OSPL_HOME)/src/api/streams/ccpp/orb/$(SPLICE_ORB)
CXXINCS += -I$(OSPL_HOME)/src/api/streams/ccpp/bld/$(SPLICE_TARGET)

$(TARGET_DLIB): $(ORB_TARGETS)

$(STREAMS_API_SRC) $(STREAMS_API_HDR): $(IDL_DIR)/$(STREAMS_API_IDL)
	$(ORB_IDL_COMPILER) $(ORB_CXX_FLAGS) $(ORB_IDL_FLAGS) $(IDL_INC_FLAGS) "$(shell $(OSPL_HOME)/bin/ospl_normalizePath $<)"

-include $(DEPENDENCIES)
