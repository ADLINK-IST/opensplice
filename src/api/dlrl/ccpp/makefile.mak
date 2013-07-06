TARGET_DLIB	:= $(DDS_DLRLCCPP)

IDL_DIR		 := $(OSPL_HOME)/src/api/dlrl/ccpp/idl
DLRL_API_IDL := dds_dlrl.idl

include $(OSPL_HOME)/setup/makefiles/orbdeps.mak

# default values for directory and idl-files to process
# include dcps idl as well because DLRL api is dependent on that file
IDL_FILES     = $(DLRL_API_IDL)
IDL_INC_FLAGS = -I$(IDL_DIR) -I$(OSPL_HOME)/etc/idl
vpath %.idl	$(IDL_DIR)

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile
# IDL_O will be linked into the final target
IDL_H = $(ORB_DLRL_API_HDR)
IDL_C = $(ORB_DLRL_API_SRC)
IDL_O = $(ORB_DLRL_API_OBJ)

include $(OSPL_HOME)/setup/makefiles/target.mak

ORB_DIR     := $(IDL_DIR)/$(SPLICE_TARGET)/$(SPLICE_ORB)
ORB_TARGETS := $(ORB_DLRL_API_OBJ)

CPPFLAGS	+= -DOSPL_BUILD_DLRLCCPP $(ORB_CPP_FLAGS)

CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS)
CXXFLAGS += $(ORB_CXX_FLAGS)
CXXINCS	+= $(ORB_INCLUDE)
CXXINCS	+= -I$(OSPL_HOME)/src/api/dcps/gapi/include
CXXINCS	+= -I$(OSPL_HOME)/src/api/dcps/gapi/code
CXXINCS	+= -I$(OSPL_HOME)/src/api/dcps/ccpp/include
CXXINCS  += -I$(OSPL_HOME)/src/api/dcps/ccpp/bld/$(SPLICE_TARGET)
CXXINCS	+= -I$(OSPL_HOME)/src/api/dlrl/ccpp/include
CXXINCS	+= -I$(OSPL_HOME)/src/loc/kernel/include
CXXINCS	+= -I$(OSPL_HOME)/src/loc/collections/include
CXXINCS	+= -I$(OSPL_HOME)/src/loc/metamodel/include
CXXINCS	+= -I$(OSPL_HOME)/src/loc/kernel/include/bridge
CXXINCS  += -I$(OSPL_HOME)/src/database/database/include
CXXINCS	+= -I$(OSPL_HOME)/src/loc/util/include
CXXINCS	+= -I$(OSPL_HOME)/src/user/include
CXXINCS	+= -I$(OSPL_HOME)/src/kernel/include
CXXINCS	+= -I$(OSPL_HOME)/src/database/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS) $(LDLIBS_CXX) $(ORB_LDLIBS) -l$(DDS_LOCKERNEL)
LDLIBS	+= -l$(DDS_LOCUTIL) -l$(DDS_LOCMETAMODEL) -l$(DDS_LOCCOLLECTIONS)
LDLIBS	+= -l$(DDS_DCPSGAPI) -l$(DDS_DCPSCCPP) -l$(DDS_USER)
LDLIBS	+= -l$(DDS_KERNEL) -l$(DDS_DATABASE) -l$(DDS_OS)

#we refer to the orb directory defined for the DCPS CPP API
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/ccpp/orb/$(SPLICE_ORB)
CXXINCS     += -I$(OSPL_HOME)/src/api/dlrl/ccpp/orb/$(SPLICE_ORB)

$(TARGET_DLIB): $(ORB_TARGETS)

$(ORB_DLRL_API_SRC) $(ORB_DLRL_API_HDR): $(IDL_DIR)/$(DLRL_API_IDL)
	$(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(IDL_INC_FLAGS) $<

-include $(DEPENDENCIES)
