TARGET_DLIB	:= $(DDS_DCPSSACPP)

# Determine import/export macro and include file.
DECL_PREFIX := SACPP_API
DECL_INCLUDE := sacpp_if.h

# Input IDL files.
IDL_DIR		:= $(OSPL_HOME)/etc/idl
vpath %.idl $(IDL_DIR)

TOPIC_IDL   := dds_dcps_builtintopics.idl
TOPIC_IDL2  := dds_builtinTopics.idl
TOPIC_IDL3  := dds_namedQosTypes.idl
INT_IDL     := dds_dcps_interfaces.idl
DCPS_IDL    := $(TOPIC_IDL:%.idl=%Dcps.idl)
DCPS_IDL2    := $(TOPIC_IDL2:%.idl=%Dcps.idl)
DCPS_API_IDL:= dds_dcps.idl

# Explicitly point to an alternative location for all required source files.
CODE_DIR	:= ../../../ccpp/code
SACPP_CODE	:= $(notdir $(wildcard ../../code/*.cpp))

CPP_FILES 	:= $(notdir $(wildcard $(CODE_DIR)/*.cpp)) $(SACPP_CODE)

# idlpp compiler settings.
IDLPP       = $(WINCMD) idlpp
IDL_INC_FLAGS= -I$(IDL_DIR)
IDLPPFLAGS  := $(IDL_INC_FLAGS) -l cpp -S
ifneq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
IDLPPFLAGS  += -P$(DECL_PREFIX),$(DECL_INCLUDE)
endif

# idlpp output
IDLPP_HDR   = ccpp_$(TOPIC_IDL:%.idl=%.h) $(TOPIC_IDL:%.idl=%Dcps_impl.h) $(TOPIC_IDL:%.idl=%SplDcps.h)
IDLPP_CPP   = $(TOPIC_IDL:%.idl=%SplDcps.cpp) $(TOPIC_IDL:%.idl=%Dcps_impl.cpp) $(TOPIC_IDL:%.idl=%Dcps.cpp)
IDLPP_IDL   = $(TOPIC_IDL:%.idl=%Dcps.idl)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

IDLPP_HDR2   = ccpp_$(TOPIC_IDL2:%.idl=%.h) $(TOPIC_IDL2:%.idl=%Dcps_impl.h) $(TOPIC_IDL2:%.idl=%SplDcps.h)
IDLPP_CPP2   = $(TOPIC_IDL2:%.idl=%SplDcps.cpp) $(TOPIC_IDL2:%.idl=%Dcps_impl.cpp) $(TOPIC_IDL2:%.idl=%Dcps.cpp)
IDLPP_IDL2   = $(TOPIC_IDL2:%.idl=%Dcps.idl)
IDLPP_OBJ2   = $(IDLPP_CPP2:%.cpp=%$(OBJ_POSTFIX))

IDLPP_HDR3   = ccpp_$(TOPIC_IDL3:%.idl=%.h) $(TOPIC_IDL3:%.idl=%Dcps_impl.h) $(TOPIC_IDL3:%.idl=%SplDcps.h)
IDLPP_CPP3   = $(TOPIC_IDL3:%.idl=%SplDcps.cpp) $(TOPIC_IDL3:%.idl=%Dcps_impl.cpp) $(TOPIC_IDL3:%.idl=%Dcps.cpp)
IDLPP_IDL3   = $(TOPIC_IDL3:%.idl=%Dcps.idl)
IDLPP_OBJ3   = $(IDLPP_CPP3:%.cpp=%$(OBJ_POSTFIX))

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H       = $(IDLPP_HDR) $(IDLPP_HDR2) $(IDLPP_HDR3)
IDL_C       = $(IDLPP_CPP) $(IDLPP_CPP2) $(IDLPP_CPP3)
IDL_O       = $(IDLPP_OBJ) $(IDLPP_OBJ2) $(IDLPP_OBJ3)

# Include the actual building rules.
include	     $(OSPL_HOME)/setup/makefiles/target.mak

# Fine tune the compiler flags.
CPPFLAGS	+= -DOSPL_BUILD_DCPSCCPP

CXXINCS		+= -I$(OSPL_HOME)/src/kernel/include
CXXINCS     += -I$(OSPL_HOME)/src/user/include
CXXINCS		+= -I$(OSPL_HOME)/src/database/database/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/sacpp/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/ccpp/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/gapi/include
CXXINCS     += -I$(OSPL_HOME)/src/api/dcps/common/include
CXXINCS     += -I$(OSPL_HOME)/src/utilities/include

CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS)

# If specific linker options for CXX are set, use those to overrule the SHLDFLAGS
ifneq (,$(SHLDCXXFLAGS))
SHLDFLAGS = $(SHLDCXXFLAGS)
endif

# Fine tune the Linker flags.
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_CORE) $(LDLIBS_CXX)

$(SACPP_CODE):
	cp ../../code/$@ .

# Generate the C++ interfaces from the IDL descriptions.
$(IDLPP_HDR) $(IDLPP_CPP) $(IDLPP_IDL) $(IDLPP_HDR2) $(IDLPP_CPP2) $(IDLPP_IDL2) $(IDLPP_HDR3) $(IDLPP_CPP3) $(IDLPP_IDL3) : $(IDL_DIR)/$(TOPIC_IDL) $(IDL_DIR)/$(TOPIC_IDL2) $(IDL_DIR)/$(TOPIC_IDL3)
	$(IDLPP) $(IDLPPFLAGS) $(IDL_DIR)/$(TOPIC_IDL)
	$(IDLPP) $(IDLPPFLAGS) $(IDL_DIR)/$(TOPIC_IDL2)
	$(IDLPP) $(IDLPPFLAGS) $(IDL_DIR)/$(TOPIC_IDL3)

$(DEPENDENCIES): $(SACPP_CODE)
