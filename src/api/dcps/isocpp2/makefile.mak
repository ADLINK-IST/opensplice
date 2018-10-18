TARGET_DLIB	:= $(DDS_DCPSISOCPP)2

# Input IDL files.
IDL_DIR     := $(OSPL_HOME)/etc/idl
vpath %.idl $(IDL_DIR)
TOPIC_IDL   := dds_IoTData.idl

# idlpp compiler settings.
IDLPP       = $(WINCMD) idlpp
IDL_INC_FLAGS= -I$(IDL_DIR)
IDLPPFLAGS  := $(IDL_INC_FLAGS) -l isocpp2

# Compile IoTData generated code
IDL_C   = dds_IoTData.cpp dds_IoTDataSplDcps.cpp
IDL_H   = $(TOPIC_IDL:%.idl=%.h) $(TOPIC_IDL:%.idl=%_DCPS.hpp) $(TOPIC_IDL:%.idl=%SplDcps.h)
IDL_O   = $(IDL_C:%.cpp=%$(OBJ_POSTFIX))

# Explicitly point to an alternative location for all required source files.
CODE_DIR    := $(OSPL_HOME)/src/api/dcps/isocpp2/code
CPP_FILES   := $(subst $(CODE_DIR)/,,$(shell find $(CODE_DIR) -name '*.cpp'))
CPP_DIRS    := $(sort $(dir $(CPP_FILES)))

# Include the actual building rules.
include	     $(OSPL_HOME)/setup/makefiles/target.mak

# Fine tune the compiler flags. Enable both the ISOCPP as the SACPP build flags, 
# as SACPP will be linked into the ISOCPP library. 
CPPFLAGS	+= -DBUILD_OMG_DDS_API -DOSPL_BUILD_DCPSCPP
CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS) $(ISOCPP2_CXX_FLAGS)

CXXINCS		+= -I$(OSPL_HOME)/src/kernel/include
CXXINCS		+= -I$(OSPL_HOME)/src/user/include
CXXINCS		+= -I$(OSPL_HOME)/src/utilities/include
CXXINCS		+= -I$(OSPL_HOME)/src/database/database/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/common/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/isocpp2/include
CXXINCS		+= -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
ifneq "$(BOOST_ROOT_UNIX)" ""
CXXINCS		+= -I$(BOOST_ROOT_UNIX)
endif

# LTO provides problems when using g++482.
ifneq ($(CFLAGS_LTO),)
CFLAGS_LTO =
endif

CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS)

# Fine tune the Linker flags.
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += -l$(DDS_CORE) $(SHLDLIBS)
LDLIBS  += $(LDLIBS_CXX)

%.cpp %.h %Dcps.h %_DCPS.hpp %SplDcps.h: %.idl
	$(IDLPP) $(IDLPPFLAGS) $<

.make_bld_dirs:
	mkdir -p $(CPP_DIRS)
	touch .make_bld_dirs

-include $(DEPENDENCIES): .make_bld_dirs
