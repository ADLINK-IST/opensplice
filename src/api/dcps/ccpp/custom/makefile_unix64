# Input IDL files.
IDL_DIR     := $(OSPL_HOME)/etc/idl
vpath %.idl $(IDL_DIR)
# pre-existing IDL files
TOPIC_IDL   := dds_dcps_builtintopics.idl
TOPIC_IDL2  := dds_builtinTopics.idl
INT_IDL     := dds_dcps_interfaces.idl
DCPS_IDL    := $(TOPIC_IDL:%.idl=%Dcps.idl)
DCPS_IDL2    := $(TOPIC_IDL2:%.idl=%Dcps.idl)
DCPS_API_IDL:= dds_dcps.idl
IDL_FILES   := $(TOPIC_IDL) $(TOPIC_IDL2) $(INT_IDL) $(DCPS_IDL) $(DCPS_API_IDL)

#Only valid for Unix/Linux type systems.
OBJ_POSTFIX := .o
INLINESRC_POSTFIX := .i

.PRECIOUS: %C.cpp %SplDcps.cpp %Dcps_impl.cpp

include ./orbdeps.mak

# This determines what will be processed

# idlpp output
IDLPP_HDR   = ccpp_$(TOPIC_IDL:%.idl=%.h) $(TOPIC_IDL:%.idl=%Dcps_impl.h) $(TOPIC_IDL:%.idl=%SplDcps.h)
IDLPP_CPP   = $(TOPIC_IDL:%.idl=%SplDcps.cpp) $(TOPIC_IDL:%.idl=%Dcps_impl.cpp)
IDLPP_IDL   = $(TOPIC_IDL:%.idl=%Dcps.idl)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

IDLPP_HDR2   = ccpp_$(TOPIC_IDL2:%.idl=%.h) $(TOPIC_IDL2:%.idl=%Dcps_impl.h) $(TOPIC_IDL2:%.idl=%SplDcps.h)
IDLPP_CPP2   = $(TOPIC_IDL2:%.idl=%SplDcps.cpp) $(TOPIC_IDL2:%.idl=%Dcps_impl.cpp)
IDLPP_IDL2   = $(TOPIC_IDL2:%.idl=%Dcps.idl)
IDLPP_OBJ2   = $(IDLPP_CPP2:%.cpp=%$(OBJ_POSTFIX))

# API Classes.
API_SRC             = $(wildcard ccpp_*.cpp)
API_OBJ             = $(API_SRC:%.cpp=%.o)

# All objects
OBJS = $(IDLPP_OBJ) $(IDLPP_OBJ2) $(ORB_TOP_OBJ) $(ORB_API_OBJ) $(API_OBJ) $(IDLPP_ORB_OBJ)

# library target name
TARGET_DLIB := dcpsccpp
DLIB_PREFIX := lib
DLIB_POSTFIX := .so

TARGET = $(DLIB_PREFIX)$(TARGET_DLIB)$(DLIB_POSTFIX)

INCLUDE += -I./
INCLUDE += -I$(OSPL_HOME)/include
INCLUDE += -I$(OSPL_HOME)/include/sys
INCLUDE += -I$(OSPL_HOME)/include/dcps/C++/CCPP
INCLUDE += -I$(OSPL_HOME)/include/dcps/C++/CCPP/$(SPLICE_ORB)
INCLUDE += $(ORB_INCLUDE)

# compiler and compiler flags (Only valid for gcc-compilers)
CXX := g++
CXXFLAGS := -m64 -g $(ORB_CXX_FLAGS) -fPIC
CPPFLAGS = $(ORB_SELECT_FLAGS) $(INCLUDE) $(ORB_CPP_FLAGS)

# linker and linker flags (Only valid for gcc-linkers)
LD_SO := $(CXX)
SPLICE_LIBRARY_PATH := $(OSPL_HOME)/lib
LD_FLAGS := -shared -fPIC -static-libgcc
LD_LIBS  := -lc -lm -lpthread -lnsl -ldcpsgapi -lddsdatabase -lddsos -lstdc++ $(ORB_LDLIBS)

# SPLICE IDL preprocessor and preprocessor flags
IDLPP := idlpp
IDLPPFLAGS := -l cpp -C -b CCPP/$(SPLICE_ORB)

#Dependencies

all : $(TARGET)

$(IDLPP_OBJ): $(IDLPP_ORB_HDR) $(ORB_TOP_HDR) $(ORB_API_HDR)

#generic rules for IDL preprocessing

# Generate the C++ interfaces from the IDL descriptions.
$(IDLPP_HDR) $(IDLPP_CPP) $(IDLPP_IDL) $(IDLPP_HDR2) $(IDLPP_CPP2) $(IDLPP_IDL2) : $(IDL_DIR)/$(TOPIC_IDL) $(IDL_DIR)/$(TOPIC_IDL2)
	$(IDLPP) $(IDLPPFLAGS) $(IDL_DIR)/$(TOPIC_IDL)
	$(IDLPP) $(IDLPPFLAGS) $(IDL_DIR)/$(TOPIC_IDL2)

$(ORB_TOP_SRC) $(ORB_TOP_HDR): $(TOPIC_IDL) $(TOPIC_IDL2)
	unset CXXINCS; unset CINCS; $(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(IDL_DIR)/$(TOPIC_IDL)
	$(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(IDL_DIR)/$(TOPIC_IDL2)

$(IDLPP_ORB_SRC) $(IDLPP_ORB_HDR): $(TOPIC_IDL:%.idl=%Dcps.idl) $(TOPIC_IDL2:%.idl=%Dcps.idl)
	unset CXXINCS; unset CINCS; $(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(TOPIC_IDL:%.idl=%Dcps.idl)
	$(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(TOPIC_IDL2:%.idl=%Dcps.idl)

$(ORB_API_SRC) $(ORB_API_HDR): $(DCPS_API_IDL) $(INT_IDL)
	unset CXXINCS; unset CINCS; $(WINCMD) $(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(IDL_DIR)/$(DCPS_API_IDL)
	$(ORB_IDL_COMPILER) $(ORB_IDL_FLAGS) $(ORB_CXX_FLAGS) $(IDL_INC_FLAGS) $(IDL_DIR)/$(INT_IDL)

$(TARGET): $(OBJS)
	$(LD_SO) -L$(SPLICE_LIBRARY_PATH) $(LD_FLAGS) $(OBJS) $(LD_LIBS) -o $(TARGET)
	-mkdir -p $(SPLICE_ORB)/include
	-mkdir -p $(SPLICE_ORB)/lib
	cp *.h *.i $(SPLICE_ORB)/include
	cp $(TARGET) $(SPLICE_ORB)/lib


clean:
	-rm $(TARGET) $(OBJS) $(ORB_TOP_SRC) $(ORB_API_SRC) $(IDLPP_IDL) $(IDLPP_SRC) $(IDLPP_ORB_SRC) $(ORB_TOP_HDR) $(ORB_API_HDR) $(IDLPP_ORB_HDR) $(IDLPP_HDR)

