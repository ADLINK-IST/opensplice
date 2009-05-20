TARGET_DLIB := $(DDS_DCPSCCPP)

# Determine import/export macro and include file.
DECL_PREFIX := OS_DCPS_API
DECL_INCLUDE := ccpp_dcps_if.h

# Import all ORB specific data.
include     $(OSPL_HOME)/setup/makefiles/orbdeps.mak

# Input IDL files.
IDL_DIR     := ../../idl
vpath %.idl $(IDL_DIR)
TOPIC_IDL   := dds_dcps_builtintopics.idl
DCPS_IDL    := $(TOPIC_IDL:%.idl=%Dcps.idl)
DCPS_API_IDL:= dds_dcps.idl
IDL_FILES   := $(TOPIC_IDL) $(DCPS_IDL) $(DCPS_API_IDL)

# idlpp compiler settings.
IDLPP       := idlpp 
IDL_INC_FLAGS= -I$(IDL_DIR) -I$(OSPL_HOME)/src/api/dcps/ccpp/idl
IDLPPFLAGS  := $(IDL_INC_FLAGS) -l cpp -C
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
IDLPPFLAGS  += -P$(DECL_PREFIX),$(DECL_INCLUDE)
endif

# idlpp output
IDLPP_HDR   = ccpp_$(TOPIC_IDL:%.idl=%.h) $(TOPIC_IDL:%.idl=%Dcps_impl.h) $(TOPIC_IDL:%.idl=%SplDcps.h) 
IDLPP_CPP   = $(TOPIC_IDL:%.idl=%SplDcps.cpp) $(TOPIC_IDL:%.idl=%Dcps_impl.cpp) 
IDLPP_IDL   = $(TOPIC_IDL:%.idl=%Dcps.idl)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

# ORB IDL compiler settings
IDLCC       := $(ORB_IDL_COMPILER)

# ORB IDL compiler output
IDLCC_H     = $(ORB_TOP_HDR) $(ORB_API_HDR) $(IDLPP_ORB_HDR)
IDLCC_CPP   = $(ORB_TOP_SRC) $(ORB_API_SRC) $(IDLPP_ORB_SRC)
IDLCC_OBJ   = $(ORB_TOP_OBJ) $(ORB_API_OBJ) $(IDLPP_ORB_OBJ)

# This determines what/how it will be processed 
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H       = $(IDLPP_HDR) $(IDLCC_H)
IDL_C       = $(IDLPP_CPP) $(IDLCC_CPP)
IDL_O       = $(IDLPP_OBJ) $(IDLCC_OBJ)

# Include the actual building rules.
include     $(OSPL_HOME)/setup/makefiles/target.mak

# Fine tune the compiler flags.
CPPFLAGS	+= -DOSPL_BUILD_DCPSCCPP

CXXINCS  += $(ORB_INCLUDE)
CXXINCS  += -I$(OSPL_HOME)/src/kernel/include
CXXINCS  += -I$(OSPL_HOME)/src/database/database/include
CXXINCS  += -I$(OSPL_HOME)/src/api/dcps/ccpp/include
CXXINCS  += -I$(OSPL_HOME)/src/api/dcps/ccpp/orb/$(SPLICE_ORB)
CXXINCS  += -I$(OSPL_HOME)/src/api/dcps/gapi/include

CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS)

# Fine tune the Linker flags.
LDFLAGS  += $(SHLDFLAGS)

LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_DCPSGAPI) -l$(DDS_DATABASE) -l$(DDS_OS) 
LDLIBS   += $(LDLIBS_CXX) $(ORB_LDLIBS)

.PHONY: make_idl_preprocessor

# Make preprocessor if not already done so.
$(addprefix $(IDL_DIR)/,$(TOPIC_IDL)): make_idl_preprocessor

make_idl_preprocessor:
	cd $(OSPL_HOME)/src/cpp; make
	cd $(OSPL_HOME)/src/tools/idlpp; make

# Generate the C++ interfaces from the IDL descriptions.
$(IDLPP_HDR) $(IDLPP_CPP) $(IDLPP_IDL): $(IDL_DIR)/$(TOPIC_IDL)
	$(IDLPP) $(IDLPPFLAGS) $<

$(ORB_TOP_SRC) $(ORB_TOP_HDR): $(TOPIC_IDL)
	unset CXXINCS; unset CINCS; $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS)  $^

$(IDLPP_ORB_SRC) $(IDLPP_ORB_HDR): $(TOPIC_IDL:%.idl=%Dcps.idl)
	unset CXXINCS; unset CINCS; $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS)  $^

$(ORB_API_SRC) $(ORB_API_HDR): $(DCPS_API_IDL)
	unset CXXINCS; unset CINCS; $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS)  $^

$(DEPENDENCIES): $(IDLPP_HDR) $(IDLCC_H)
