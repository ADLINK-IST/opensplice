TARGET_DLIB	:= $(DDS_DCPSISOCPP)

# Explicitly point to an alternative location for all required source files.
CODE_DIR    := $(OSPL_HOME)/src/api/dcps/isocpp/code
CPP_FILES   := $(subst $(CODE_DIR)/,,$(shell find $(CODE_DIR) -name *.cpp))
CPP_DIRS    := $(sort $(dir $(CPP_FILES)))

# Specify the SACPP Objects that need to be linked in.
EXTRAOBJECTS = $(wildcard ../../../c++/sacpp/bld/$(SPLICE_TARGET)/*$(OBJ_POSTFIX))

# Include the actual building rules.
include	     $(OSPL_HOME)/setup/makefiles/target.mak

# LTO must be disabled for now (OSPL-6421)
ifneq ($(CFLAGS_LTO),)
CFLAGS_LTO =
endif

# Fine tune the compiler flags. Enable both the ISOCPP as the SACPP build flags,
# as SACPP will be linked into the ISOCPP library.
CPPFLAGS	+= -DBUILD_OMG_DDS_API -DOSPL_BUILD_DCPSCPP

CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS)

CXXINCS		+= -I$(OSPL_HOME)/src/kernel/include
CXXINCS		+= -I$(OSPL_HOME)/src/user/include
CXXINCS		+= -I$(OSPL_HOME)/src/utilities/include
CXXINCS		+= -I$(OSPL_HOME)/src/database/database/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/common/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/c++/sacpp/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/c++/sacpp/bld/$(SPLICE_TARGET)
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/c++/common/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/isocpp/include
ifneq "$(BOOST_ROOT_UNIX)" ""
CXXINCS		+= -I$(BOOST_ROOT_UNIX)
endif

CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS)

# Fine tune the Linker flags.
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += -l$(DDS_CORE) $(SHLDLIBS)
LDLIBS  += $(LDLIBS_CXX)

.make_bld_dirs:
	mkdir -p $(CPP_DIRS)
	touch .make_bld_dirs

-include $(DEPENDENCIES): .make_bld_dirs
