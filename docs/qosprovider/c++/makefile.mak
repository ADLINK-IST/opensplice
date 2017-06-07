# Specify target name and target location.
TARGET_EXEC     := qosprovider_cpp
TARGET_LINK_DIR := ../../exec/$(SPLICE_TARGET)


# Now include the generic rules for building CCPP test applications.
include     $(OSPL_HOME)/setup/makefiles/target.mak

LD_EXE = $(LD_CXX)


# Fine tune the compiler flags.
CFLAGS   += $(MTCFLAGS)

CXXINCS += -I$(OSPL_HOME)/src/kernel/include
CXXINCS += -I$(OSPL_HOME)/src/user/include
CXXINCS += -I$(OSPL_HOME)/src/database/database/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/c++/common/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/c++/sacpp/include
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/c++/sacpp/bld/$(SPLICE_TARGET)

# Fine tune the Linker flags.
LDLIBS   += -l$(DDS_DCPSSACPP) -l$(DDS_CORE)
