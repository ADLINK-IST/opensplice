.NOTPARALLEL:

TARGET_DLIB	:= $(DDS_STREAMSSACPP)

# Streams API IDL
IDL_DIR			:= $(OSPL_HOME)/src/api/streams/idl
STREAMS_API_IDL	:= streams_dcps.idl

# Use ccpp code dir
CODE_DIR	:= ../../../ccpp/code
SACPP_SRC	:= streams_dcps.cpp
CPP_FILES	:= $(notdir $(wildcard $(CODE_DIR)/*.cpp)) $(SACPP_SRC)

# cppgen
CPPGEN		:= cppgen

# Include the actual building rules.
include		$(OSPL_HOME)/setup/makefiles/target.mak

DECL_PREFIX := OS_STREAMS_API
DECL_INCLUDE := ccpp_streams_if.h

# Fine tune the compiler/linker flags.
CPPGENFLAGS	:= -I"$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_HOME)/etc/idl)"
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
CPPGENFLAGS += -import_export=$(DECL_PREFIX),$(DECL_INCLUDE)
endif

CPPFLAGS	+= -DOSPL_BUILD_STREAMSSACPP
CXXFLAGS	+= $(SHCFLAGS)

# If specific linker options for CXX are set, use those to overrule the SHLDFLAGS
ifneq (,$(SHLDCXXFLAGS))
SHLDFLAGS = $(SHLDCXXFLAGS)
endif


# Fine tune the Linker flags.
LDFLAGS	+= $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS) $(LDLIBS_CXX)
LDLIBS	+= -l$(DDS_DCPSSACPP) -l$(DDS_CORE)

# Use ccpp include dir
CXXINCS		+= -I$(OSPL_HOME)/src/api/streams/ccpp/include

# Add includes for DCPS API
CXXINCS     += -I$(OSPL_HOME)/src/api/dcps/c++/common/include
CXXINCS     += -I$(OSPL_HOME)/src/api/dcps/c++/sacpp/include
CXXINCS     += -I$(OSPL_HOME)/src/api/dcps/c++/sacpp/bld/$(SPLICE_TARGET)
CXXINCS     += -I$(OSPL_HOME)/src/database/database/include
CXXINCS     += -I$(OSPL_HOME)/src/kernel/include
CXXINCS     += -I$(OSPL_HOME)/src/user/include

$(SACPP_SRC): $(IDL_DIR)/$(STREAMS_API_IDL)
	$(CPPGEN) $(CPPGENFLAGS) "$(shell $(OSPL_HOME)/bin/ospl_normalizePath $<)"
# 	Copy header files to include directory so they are picked up by the regular include paths and when building HDE/RTS
	@cp streams_dcps.h "$(OSPL_HOME)/src/api/streams/sacpp/include"
	@cp "$(OSPL_HOME)/src/api/streams/ccpp/include/streams_dcpsSplDcps.h"  "$(OSPL_HOME)/src/api/streams/sacpp/include"
	@cp "$(OSPL_HOME)/src/api/streams/ccpp/include/ccpp_streams_if.h" "$(OSPL_HOME)/src/api/streams/sacpp/include"

$(DEPENDENCIES): $(SACPP_SRC)
