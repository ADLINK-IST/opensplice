TARGET_DLIB	:= $(DDS_FACECPP)

# Explicitly point to an alternative location for all required source files.
CODE_DIR    := $(OSPL_HOME)/src/api/face/cpp/code
CPP_FILES   := $(subst $(CODE_DIR)/,,$(shell find $(CODE_DIR) -name *.cpp))
CPP_DIRS    := $(sort $(dir $(CPP_FILES)))

# Include the actual building rules.
include	     $(OSPL_HOME)/setup/makefiles/target.mak

# Fine tune the compiler flags. Enable both the ISOCPP as the SACPP build flags,
# as both SACPP and ISOCPP will be linked in the FaceCpp library.
CPPFLAGS	+= -DBUILD_VORTEX_FACE_API -DOSPL_BUILD_DCPSCPP
CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS) $(ISOCPP2_CXX_FLAGS)

CXXINCS		+= -I$(OSPL_HOME)/src/kernel/include
CXXINCS		+= -I$(OSPL_HOME)/src/user/include
CXXINCS		+= -I$(OSPL_HOME)/src/utilities/include
CXXINCS		+= -I$(OSPL_HOME)/src/configuration/config/include
CXXINCS		+= -I$(OSPL_HOME)/src/configuration/parser/include
CXXINCS		+= -I$(OSPL_HOME)/src/database/database/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/common/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/isocpp2/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/face/cpp/include
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
LDLIBS  += -l$(DDS_CORE) $(SHLDLIBS) -l$(DDS_DCPSISOCPP)2
LDLIBS  += $(LDLIBS_CXX)

.make_bld_dirs:
	mkdir -p $(CPP_DIRS)
	touch .make_bld_dirs

-include $(DEPENDENCIES): .make_bld_dirs
