TARGET_DLIB := $(DDS_DLRLSACPP)

# Determine import/export macro and include file.
DECL_PREFIX := OS_DLRL_API
DECL_INCLUDE := ccpp_dlrl_if.h

# Input IDL files.
IDL_DIR       := ../../../ccpp/idl
DLRL_API_IDL  := dds_dlrl.idl

ifneq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
IDL_INC_FLAGS  = -I'$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(IDL_DIR))' -I'$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_HOME)/etc/idl)'
else
IDL_INC_FLAGS  = -I$(IDL_DIR) -I$(OSPL_HOME)/etc/idl
endif

# Explicitly point to an alternative location for all required source files.
CODE_DIR              :=../../../ccpp/code
SACPP_FILES           := sacpp_ValueBase.cpp sacpp_DefaultValueRefCountBase.cpp sacpp_Exception.cpp sacpp_UserException.cpp sacpp_ExceptionInitializer.cpp
DB_DLRL_API_SRC       := $(DLRL_API_IDL:%.idl=%SplDcps.cpp)
CPP_FILES             := $(notdir $(wildcard $(CODE_DIR)/*.cpp)) $(DB_DLRL_API_SRC) $(SACPP_FILES) $(DB_DLRL_API_SRC)

# idlpp compiler settings.
IDLPP           := idlpp
IDLPPFLAGS	    := $(IDL_INC_FLAGS) -l cpp -S

# cppgen compiler settings.
CPPGEN          := cppgen
CPPGENFLAGS	    := $(IDL_INC_FLAGS)
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
IDLPPFLAGS  += -P $(DECL_PREFIX),$(DECL_INCLUDE)
CPPGENFLAGS  += -import_export=OS_DLRL_API,$(DECL_INCLUDE)
endif

# idlpp output
IDLPP_HDR   = $(DLRL_API_IDL:%.idl=%.h) $(DLRL_API_IDL:%.idl=%Dcps_impl.h) $(DLRL_API_IDL:%.idl=%SplDcps.h)
IDLPP_CPP   = $(DLRL_API_IDL:%.idl=%.cpp) $(DLRL_API_IDL:%.idl=%SplDcps.cpp) $(DLRL_API_IDL:%.idl=%Dcps_impl.cpp) $(DLRL_API_IDL:%.idl=%Dcps.cpp)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H       = $(IDLPP_HDR)
IDL_C       = $(IDLPP_CPP)
IDL_O       = $(IDLPP_OBJ)

# Include the actual building rules.
include		$(OSPL_HOME)/setup/makefiles/target.mak

# Fine tune the compiler flags.
CPPFLAGS	+= -DOSPL_BUILD_DLRLCCPP

# Set includes for API
CXXINCS		+= -I$(OSPL_HOME)/src/database/database/include
CXXINCS		+= -I$(OSPL_HOME)/src/kernel/include
CXXINCS		+= -I$(OSPL_HOME)/src/user/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/gapi/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/gapi/code
CXXINCS	    += -I$(OSPL_HOME)/src/api/dlrl/ccpp/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/ccpp/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/sacpp/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/sacpp/bld/$(SPLICE_TARGET)
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/sac/include
CXXINCS   	+= -I$(OSPL_HOME)/src/loc/kernel/include
CXXINCS	    += -I$(OSPL_HOME)/src/loc/collections/include
CXXINCS  	+= -I$(OSPL_HOME)/src/loc/metamodel/include
CXXINCS 	+= -I$(OSPL_HOME)/src/loc/kernel/include/bridge
CXXINCS 	+= -I$(OSPL_HOME)/src/loc/util/include

CXXFLAGS	+= $(SHCFLAGS) $(MTCFLAGS)

# Fine tune the Linker flags.
LDFLAGS	+= $(SHLDFLAGS)
LDLIBS	+= $(SHLDLIBS)
LDLIBS	+= $(LDLIBS_CXX) -l$(DDS_LOCKERNEL) -l$(DDS_LOCUTIL)
LDLIBS	+= -l$(DDS_LOCMETAMODEL) -l$(DDS_LOCCOLLECTIONS) -l$(DDS_DCPSGAPI)
LDLIBS	+= -l$(DDS_DCPSSACPP) -l$(DDS_USER) -l$(DDS_KERNEL) -l$(DDS_DATABASE)
LDLIBS	+= -l$(DDS_OS)


.PHONY: make_idl_preprocessor

$(addprefix $(IDL_DIR)/,$(DLRL_API_IDL)): make_idl_preprocessor

make_idl_preprocessor:
	cd $(OSPL_HOME)/src/cpp; make
	cd $(OSPL_HOME)/src/tools/cppgen; make
	cd $(OSPL_HOME)/src/tools/idlpp; make

$(SACPP_FILES):
	cp $(OSPL_HOME)/src/api/dcps/sacpp/code/sacpp_ValueBase.cpp .
	cp $(OSPL_HOME)/src/api/dcps/sacpp/code/sacpp_DefaultValueRefCountBase.cpp .
	cp $(OSPL_HOME)/src/api/dcps/sacpp/code/sacpp_Exception.cpp .
	cp $(OSPL_HOME)/src/api/dcps/sacpp/code/sacpp_UserException.cpp .
	cp $(OSPL_HOME)/src/api/dcps/sacpp/code/sacpp_ExceptionInitializer.cpp .

$(IDLPP_HDR) $(IDLPP_CPP): $(IDL_DIR)/$(DLRL_API_IDL)
	$(IDLPP) $(IDLPPFLAGS) $<
	$(CPPGEN) $(CPPGENFLAGS) $<

$(DEPENDENCIES): $(SACPP_FILES)
