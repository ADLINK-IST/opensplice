# Specify target name and target location.
TARGET_EXEC     := qosprovider_isocpp
TARGET_LINK_DIR := ../../exec/$(SPLICE_TARGET)

# Now include the generic rules for building SACPP test applications.
include     $(OSPL_HOME)/setup/makefiles/target.mak

LD_EXE = $(LD_CXX)

# Fine tune the compiler flags.
CFLAGS   += $(MTCFLAGS)

CXXINCS += -I$(OSPL_HOME)/src/api/dcps/isocpp/include/
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/c++/common/include/
CXXINCS	+= -I$(OSPL_HOME)/src/api/dcps/c++/sacpp/include
CXXINCS	+= -I$(OSPL_HOME)/src/api/dcps/c++/sacpp/bld/$(SPLICE_TARGET)
CXXINCS	+= -I$(OSPL_HOME)/src/api/dcps/common/include
CXXINCS += -I$(OSPL_HOME)/src/user/include
CXXINCS	+= -I$(OSPL_HOME)/src/kernel/include
CXXINCS	+= -I$(OSPL_HOME)/src/database/database/include
ifneq "$(BOOST_ROOT_UNIX)" ""
CXXINCS		+= -I$(BOOST_ROOT_UNIX)
endif

# Fine tune the Linker flags.
LDLIBS      += -l$(DDS_DCPSISOCPP) -l$(DDS_CORE)
