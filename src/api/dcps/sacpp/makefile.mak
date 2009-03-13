TARGET_DLIB	:= $(DDS_DCPSSACPP)

# Determine import/export macro and include file.
DECL_PREFIX := OS_DCPS_API
DECL_INCLUDE := ccpp_dcps_if.h

# Import all ORB specific data.
include      $(OSPL_HOME)/setup/makefiles/orbdeps_sa.mak

# Input IDL files.
IDL_DIR		:= ../../../ccpp/idl
vpath %.idl $(IDL_DIR)
TOPIC_IDL   := dds_dcps_builtintopics.idl
DCPS_API_IDL:= dds_dcps.idl

# Explicitly point to an alternative location for all required source files.
CODE_DIR    := ../../../ccpp/code
EORB_COBJ	:= common_cobject.cpp
EORB_LOBJ	:= CORBA_LocalObject.cpp

CPP_FILES 	:= $(notdir $(wildcard $(CODE_DIR)/*.cpp)) $(EORB_COBJ) $(EORB_LOBJ) $(ORB_API_SRC) $(ORB_TOP_SRC) $(IDLPP_ORB_SRC)


# idlpp compiler settings.
IDLPP       := idlpp 
IDL_INC_FLAGS= -I$(IDL_DIR) -I$(OSPL_HOME)/src/api/dcps/ccpp/idl
IDLPPFLAGS  := $(IDL_INC_FLAGS) -l cpp -S
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
include	     $(OSPL_HOME)/setup/makefiles/target.mak

# Fine tune the compiler flags.
CPPFLAGS	+= -DOSPL_BUILD_DCPSCCPP $(ORB_SA_CPP_FLAGS)

INCLUDE		+= $(ORB_SA_INCLUDE)
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/api/dcps/ccpp/include

ifneq (,$(wildcard $(OSPL_HOME)/src/api/dcps/sacpp/orb/$(SPLICE_SA_ORB)))
INCLUDE		+= -I$(OSPL_HOME)/src/api/dcps/sacpp/orb/$(SPLICE_SA_ORB)
else
INCLUDE		+= -I$(OSPL_OUTER_HOME)/src/api/dcps/sacpp/orb/$(SPLICE_SA_ORB)
endif

ifneq (,$(wildcard $(OSPL_HOME)/src/api/dcps/sacpp/orb_pa/$(PROC).$(OS)$(OS_REV)))
INCLUDE		+= -I$(OSPL_HOME)/src/api/dcps/sacpp/orb_pa/$(PROC).$(OS)$(OS_REV)
else
INCLUDE		+= -I$(OSPL_OUTER_HOME)/src/api/dcps/sacpp/orb_pa/$(PROC).$(OS)$(OS_REV)
endif

INCLUDE		+= -I$(OSPL_HOME)/src/api/dcps/gapi/include

CXXFLAGS    += $(SHCFLAGS) $(MTCFLAGS) 

# Fine tune the Linker flags.
LDFLAGS     += $(SHLDFLAGS)

LDLIBS      += $(SHLDLIBS)
LDLIBS      += -l$(DDS_DCPSGAPI) -l$(DDS_DATABASE) -l$(DDS_OS) $(LDLIBS_CXX) $(ORB_SA_LDLIBS)

.PHONY: make_idl_preprocessor

$(addprefix $(IDL_DIR)/,$(TOPIC_IDL)): make_idl_preprocessor

make_idl_preprocessor:
	cd $(OSPL_HOME)/src/cpp; make
	cd $(OSPL_HOME)/src/tools/idlpp; make

$(EORB_COBJ): $(EORBHOME)/src/$(EORB_COBJ)
	cp $(EORBHOME)/src/$(EORB_COBJ) $(EORB_COBJ)

$(EORB_LOBJ): $(EORBHOME)/src/$(EORB_LOBJ)
	cp $(EORBHOME)/src/$(EORB_LOBJ) $(EORB_LOBJ)

$(ORB_API_SRC) $(ORB_API_HDR): $(IDL_DIR)/$(DCPS_API_IDL)
	$(ORB_SA_IDL_COMPILER) $(ORB_SA_IDL_FLAGS) $(ORB_SA_CXX_FLAGS) $(ORB_SA_CPP_FLAGS) $<

$(IDLPP_HDR) $(IDLPP_CPP) $(ORB_TOP_HDR) $(ORB_TOP_SRC) $(IDLPP_ORB_HDR) $(IDLPP_ORB_SRC): $(IDL_DIR)/$(TOPIC_IDL)
	$(IDLPP) $(IDLPPFLAGS) $<
	
$(DEPENDENCIES): $(EORB_COBJ) $(EORB_LOBJ) $(IDLCC_H)