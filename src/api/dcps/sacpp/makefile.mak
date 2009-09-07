TARGET_DLIB	:= $(DDS_DCPSSACPP)

# Determine import/export macro and include file.
DECL_PREFIX := SACPP_API
DECL_INCLUDE := sacpp_if.h

# Input IDL files.
IDL_DIR		:= ../../../ccpp/idl
vpath %.idl $(IDL_DIR)
TOPIC_IDL	:= dds_dcps_builtintopics.idl
DCPS_API_IDL	:= dds_dcps.idl

# Explicitly point to an alternative location for all required source files.
CODE_DIR	:= ../../../ccpp/code
SACPP_CODE	:= $(notdir $(wildcard ../../code/*.cpp))

CPP_FILES 	:= $(notdir $(wildcard $(CODE_DIR)/*.cpp)) $(SACPP_CODE) 

# idlpp compiler settings.
IDLPP       := idlpp 
IDL_INC_FLAGS= -I$(IDL_DIR) -I$(OSPL_HOME)/src/api/dcps/ccpp/idl
IDLPPFLAGS  := $(IDL_INC_FLAGS) -l cpp -S
ifneq (,$(findstring win32,$(SPLICE_HOST)))
IDLPPFLAGS  += -P$(DECL_PREFIX),$(DECL_INCLUDE)
endif

# idlpp output
IDLPP_HDR   = ccpp_$(TOPIC_IDL:%.idl=%.h) $(TOPIC_IDL:%.idl=%Dcps_impl.h) $(TOPIC_IDL:%.idl=%SplDcps.h) 
IDLPP_CPP   = $(TOPIC_IDL:%.idl=%SplDcps.cpp) $(TOPIC_IDL:%.idl=%Dcps_impl.cpp) $(TOPIC_IDL:%.idl=%Dcps.cpp)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

# This determines what/how it will be processed 
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H       = $(IDLPP_HDR)
IDL_C       = $(IDLPP_CPP)
IDL_O       = $(IDLPP_OBJ)

# Include the actual building rules.
include	     $(OSPL_HOME)/setup/makefiles/target.mak

# Fine tune the compiler flags.
CPPFLAGS	+= -DOSPL_BUILD_DCPSCCPP

CXXINCS		+= -I$(OSPL_HOME)/src/kernel/include
CXXINCS		+= -I$(OSPL_HOME)/src/database/database/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/sacpp/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/ccpp/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/gapi/include

CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS) 

# Fine tune the Linker flags.
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_DCPSGAPI) -l$(DDS_DATABASE) -l$(DDS_OS) 
LDLIBS  += $(LDLIBS_CXX)

.PHONY: make_idl_preprocessor

$(addprefix $(IDL_DIR)/,$(TOPIC_IDL)): make_idl_preprocessor

make_idl_preprocessor:
	cd $(OSPL_HOME)/src/cpp; make
	cd $(OSPL_HOME)/src/tools/cppgen; make
	cd $(OSPL_HOME)/src/tools/idlpp; make

$(SACPP_CODE):
	cp ../../code/$@ .

$(IDLPP_HDR) $(IDLPP_CPP): $(IDL_DIR)/$(TOPIC_IDL)
	$(IDLPP) $(IDLPPFLAGS) $<

$(DEPENDENCIES): $(SACPP_CODE)
