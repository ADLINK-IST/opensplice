TARGET_DLIB := $(DDS_DCPSCCPP)

# Determine import/export macro and include file.
DECL_PREFIX := OS_DCPS_API
DECL_INCLUDE := ccpp_dcps_if.h

# Import all ORB specific data.
include     $(OSPL_HOME)/setup/makefiles/orbdeps.mak

# Input IDL files.
IDL_DIR     := $(OSPL_HOME)/etc/idl
vpath %.idl $(IDL_DIR)
TOPIC_IDL   := dds_dcps_builtintopics.idl
TOPIC_IDL2  := dds_builtinTopics.idl
TOPIC_IDL3  := dds_namedQosTypes.idl
INT_IDL     := dds_dcps_interfaces.idl
DCPS_IDL    := $(TOPIC_IDL:%.idl=%Dcps.idl)
DCPS_IDL2    := $(TOPIC_IDL2:%.idl=%Dcps.idl)
DCPS_API_IDL:= dds_dcps.idl
IDL_FILES   := $(TOPIC_IDL) $(TOPIC_IDL2) $(TOPIC_IDL3) $(INT_IDL) $(DCPS_IDL) $(DCPS_API_IDL)

# idlpp compiler settings.
IDLPP       = $(WINCMD) idlpp
IDL_INC_FLAGS= -I$(IDL_DIR)
IDLPPFLAGS  := $(IDL_INC_FLAGS) -l cpp -C
ifneq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
IDLPPFLAGS  += -P$(DECL_PREFIX),$(DECL_INCLUDE)
endif

# idlpp output
IDLPP_HDR   = ccpp_$(TOPIC_IDL:%.idl=%.h) $(TOPIC_IDL:%.idl=%Dcps_impl.h) $(TOPIC_IDL:%.idl=%SplDcps.h)
IDLPP_CPP   = $(TOPIC_IDL:%.idl=%SplDcps.cpp) $(TOPIC_IDL:%.idl=%Dcps_impl.cpp)
IDLPP_IDL   = $(TOPIC_IDL:%.idl=%Dcps.idl)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

IDLPP_HDR2   = ccpp_$(TOPIC_IDL2:%.idl=%.h) $(TOPIC_IDL2:%.idl=%Dcps_impl.h) $(TOPIC_IDL2:%.idl=%SplDcps.h)
IDLPP_CPP2   = $(TOPIC_IDL2:%.idl=%SplDcps.cpp) $(TOPIC_IDL2:%.idl=%Dcps_impl.cpp)
IDLPP_IDL2   = $(TOPIC_IDL2:%.idl=%Dcps.idl)
IDLPP_OBJ2   = $(IDLPP_CPP2:%.cpp=%$(OBJ_POSTFIX))

IDLPP_HDR3   = ccpp_$(TOPIC_IDL3:%.idl=%.h) $(TOPIC_IDL3:%.idl=%Dcps_impl.h) $(TOPIC_IDL3:%.idl=%SplDcps.h)
IDLPP_CPP3   = $(TOPIC_IDL3:%.idl=%SplDcps.cpp) $(TOPIC_IDL3:%.idl=%Dcps_impl.cpp)
IDLPP_IDL3   = $(TOPIC_IDL3:%.idl=%Dcps.idl)
IDLPP_OBJ3   = $(IDLPP_CPP3:%.cpp=%$(OBJ_POSTFIX))

# ORB IDL compiler settings
IDLCC       := $(ORB_IDL_COMPILER)

# ORB IDL compiler output
IDLCC_H     = $(ORB_TOP_HDR) $(ORB_API_HDR) $(IDLPP_ORB_HDR)
IDLCC_CPP   = $(ORB_TOP_SRC) $(ORB_API_SRC) $(IDLPP_ORB_SRC)
IDLCC_OBJ   = $(ORB_TOP_OBJ) $(ORB_API_OBJ) $(IDLPP_ORB_OBJ)

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H       = $(IDLPP_HDR) $(IDLPP_HDR2) $(IDLPP_HDR3) $(IDLCC_H)
IDL_C       = $(IDLPP_CPP) $(IDLPP_CPP2) $(IDLPP_CPP3) $(IDLCC_CPP)
IDL_O       = $(IDLPP_OBJ) $(IDLPP_OBJ2) $(IDLPP_OBJ3) $(IDLCC_OBJ)

# Include the actual building rules.
include     $(OSPL_HOME)/setup/makefiles/target.mak

# Fine tune the compiler flags.
CPPFLAGS	+= -DOSPL_BUILD_DCPSCCPP

CXXINCS  += $(ORB_INCLUDE)
CXXINCS  += -I$(OSPL_HOME)/src/kernel/include
CXXINCS  += -I$(OSPL_HOME)/src/user/include
CXXINCS  += -I$(OSPL_HOME)/src/database/database/include
CXXINCS  += -I$(OSPL_HOME)/src/api/dcps/ccpp/include
CXXINCS  += -I$(OSPL_HOME)/src/api/dcps/ccpp/orb/$(SPLICE_ORB)
CXXINCS  += -I$(OSPL_HOME)/src/api/dcps/gapi/include
CXXINCS  += -I$(OSPL_HOME)/src/api/dcps/common/include
CXXINCS  += -I$(OSPL_HOME)/src/utilities/include

CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS)

# If specific linker options for CXX are set, use those to overrule the SHLDFLAGS
ifneq (,$(SHLDCXXFLAGS))
SHLDFLAGS = $(SHLDCXXFLAGS)
endif

# Fine tune the Linker flags.
LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_CORE) $(LDLIBS_CXX) $(ORB_LDLIBS)

#.PHONY: make_idl_preprocessor

# Make preprocessor if not already done so.
#$(addprefix $(IDL_DIR)/,$(TOPIC_IDL)): make_idl_preprocessor

#make_idl_preprocessor:
#	cd $(OSPL_HOME)/src/cpp; make
#	cd $(OSPL_HOME)/src/tools/idlpp; make
#

# Generate the C++ interfaces from the IDL descriptions.
$(IDLPP_HDR) $(IDLPP_CPP) $(IDLPP_IDL) $(IDLPP_HDR2) $(IDLPP_CPP2) $(IDLPP_IDL2) $(IDLPP_HDR3) $(IDLPP_CPP3) $(IDLPP_IDL3) : $(IDL_DIR)/$(TOPIC_IDL) $(IDL_DIR)/$(TOPIC_IDL2) $(IDL_DIR)/$(TOPIC_IDL3)
	$(IDLPP) $(IDLPPFLAGS) $(IDL_DIR)/$(TOPIC_IDL)
	$(IDLPP) $(IDLPPFLAGS) $(IDL_DIR)/$(TOPIC_IDL2)
	$(IDLPP) $(IDLPPFLAGS) $(IDL_DIR)/$(TOPIC_IDL3)

$(ORB_TOP_SRC) $(ORB_TOP_HDR): $(TOPIC_IDL) $(TOPIC_IDL2) $(TOPIC_IDL3)
	unset CXXINCS; unset CINCS; $(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(IDL_DIR)/$(TOPIC_IDL)
	 $(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(IDL_DIR)/$(TOPIC_IDL2)
	 $(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(IDL_DIR)/$(TOPIC_IDL3)

$(IDLPP_ORB_SRC) $(IDLPP_ORB_HDR): $(TOPIC_IDL:%.idl=%Dcps.idl) $(TOPIC_IDL2:%.idl=%Dcps.idl) $(TOPIC_IDL3:%.idl=%Dcps.idl)
	unset CXXINCS; unset CINCS; $(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(TOPIC_IDL:%.idl=%Dcps.idl)
	$(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(TOPIC_IDL2:%.idl=%Dcps.idl)
	$(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(TOPIC_IDL3:%.idl=%Dcps.idl)

$(ORB_API_SRC) $(ORB_API_HDR): $(DCPS_API_IDL) $(INT_IDL)
	unset CXXINCS; unset CINCS; $(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(IDL_DIR)/$(DCPS_API_IDL)
	$(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(IDL_DIR)/$(INT_IDL)

-include $(DEPENDENCIES)
