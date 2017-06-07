TARGET_DLIB	:= $(DDS_DCPSSACPP)

# Determine import/export macro and include file.
DECL_PREFIX := OS_API
DECL_INCLUDE := cpp_dcps_if.h

# Explicitly point to an alternative location for all required source files.
CODE_DIR	:= ../../../common/code
SACPP_CODE	:= $(notdir $(wildcard ../../code/*.cpp))

CPP_FILES 	:= $(notdir $(wildcard $(CODE_DIR)/*.cpp)) $(SACPP_CODE)
vpath %.cpp ../../code ../../../common/code

# Input IDL files.
IDL_DIR     := $(OSPL_HOME)/etc/idl
vpath %.idl $(IDL_DIR)
TOPIC_IDL   := dds_builtinTopics.idl dds_dcps_builtintopics.idl dds_namedQosTypes.idl

# idlpp compiler settings.
IDLPP       = $(WINCMD) idlpp 
IDL_INC_FLAGS= -I$(IDL_DIR)
IDLPPFLAGS  := $(IDL_INC_FLAGS) -N -l c++ -S
ifneq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
IDLPPFLAGS  += -P$(DECL_PREFIX),$(DECL_INCLUDE)
endif

# idlpp output
IDL_C   = $(TOPIC_IDL:%.idl=%.cpp) $(TOPIC_IDL:%.idl=%Dcps.cpp) $(TOPIC_IDL:%.idl=%Dcps_impl.cpp) $(TOPIC_IDL:%.idl=%SplDcps.cpp)
IDL_H   = $(IDL_C:%.cpp=%.h)
IDL_O   = $(IDL_C:%.cpp=%$(OBJ_POSTFIX))

# Include the actual building rules.
include	     $(OSPL_HOME)/setup/makefiles/target.mak

# Fine tune the compiler flags.
CPPFLAGS	+= -DOSPL_BUILD_DCPSCPP

CXXINCS		+= -I$(OSPL_HOME)/src/kernel/include
CXXINCS		+= -I$(OSPL_HOME)/src/user/include
CXXINCS		+= -I$(OSPL_HOME)/src/utilities/include
CXXINCS		+= -I$(OSPL_HOME)/src/database/database/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/common/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/c++/sacpp/include
CXXINCS		+= -I$(OSPL_HOME)/src/api/dcps/c++/common/include

CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS)

# Fine tune the Linker flags.
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += -l$(DDS_CORE) $(SHLDLIBS)
LDLIBS  += $(LDLIBS_CXX)

# Avoid dependency between pre-packaged dds_dcps.cpp dds_dcps.idl.
../../code/dds_dcps.cpp:

# Generate the C# interfaces from the IDL descriptions.
%.h %.cpp %Dcps.h %Dcps.cpp %Dcps_impl.h %Dcps_impl.cpp %SplDcps.h %SplDcps.cpp: %.idl
	$(IDLPP) $(IDLPPFLAGS) $<
	
-include $(DEPENDENCIES)
