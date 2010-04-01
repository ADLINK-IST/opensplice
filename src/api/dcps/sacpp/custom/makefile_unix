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

# Only valid for Unix/Linux type systems.
OBJ_POSTFIX := .o

.PRECIOUS: %SplDcps.cpp %Dcps_impl.cpp

# idlpp output
IDLPP_HDR   = ccpp_$(TOPIC_IDL:%.idl=%.h) $(TOPIC_IDL:%.idl=%Dcps_impl.h) $(TOPIC_IDL:%.idl=%SplDcps.h)
IDLPP_CPP   = $(TOPIC_IDL:%.idl=%SplDcps.cpp) $(TOPIC_IDL:%.idl=%Dcps_impl.cpp) $(TOPIC_IDL:%.idl=%Dcps.cpp)
IDLPP_IDL   = $(TOPIC_IDL:%.idl=%Dcps.idl)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

IDLPP_HDR2   = ccpp_$(TOPIC_IDL2:%.idl=%.h) $(TOPIC_IDL2:%.idl=%Dcps_impl.h) $(TOPIC_IDL2:%.idl=%SplDcps.h)
IDLPP_CPP2   = $(TOPIC_IDL2:%.idl=%SplDcps.cpp) $(TOPIC_IDL2:%.idl=%Dcps_impl.cpp) $(TOPIC_IDL2:%.idl=%Dcps.cpp)
IDLPP_IDL2   = $(TOPIC_IDL2:%.idl=%Dcps.idl)
IDLPP_OBJ2   = $(IDLPP_CPP2:%.cpp=%$(OBJ_POSTFIX))

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H       = $(IDLPP_HDR) $(IDLPP_HDR2)
IDL_C       = $(IDLPP_CPP) $(IDLPP_CPP2)
IDL_O       = $(IDLPP_OBJ) $(IDLPP_OBJ2)

# API Classes.
API_SRC          = $(notdir $(wildcard *.cpp))
API_OBJ          = $(API_SRC:%.cpp=%.o)

# All objects
OBJS = $(IDLPP_OBJ) $(IDLPP_OBJ2) $(API_OBJ)

# library target name
TARGET_DLIB := dcpssacpp
DLIB_PREFIX := lib
DLIB_POSTFIX := .so

TARGET = $(DLIB_PREFIX)$(TARGET_DLIB)$(DLIB_POSTFIX)

INCLUDE += -I./
INCLUDE += -I$(OSPL_HOME)/include
INCLUDE += -I$(OSPL_HOME)/include/sys
INCLUDE += -I$(OSPL_HOME)/include/dcps/C++/SACPP

# compiler and compiler flags (Only valid for gcc-compilers)
CXX := g++
CXXFLAGS := -g -fPIC
CPPFLAGS = $(INCLUDE)

# linker and linker flags (Only valid for gcc-linkers)
LD_SO := $(CXX)
SPLICE_LIBRARY_PATH := $(OSPL_HOME)/lib
LD_FLAGS := -shared -fPIC -static-libgcc
LD_LIBS  := -lc -lm -lpthread -lnsl -ldcpsgapi -lddsdatabase -lddsos -lstdc++

# SPLICE IDL preprocessor and preprocessor flags
IDLPP := idlpp

ifneq (,$(findstring win32,$(SPLICE_TARGET)))
IDLPPFLAGS := -P SACPP_API -S -l cpp
else
IDLPPFLAGS := -S -l cpp
endif

#Dependencies

all : $(TARGET)

#generic rules for IDL preprocessing

# Generate the C++ interfaces from the IDL descriptions.
$(IDLPP_HDR) $(IDLPP_CPP) $(IDLPP_IDL) $(IDLPP_HDR2) $(IDLPP_CPP2) $(IDLPP_IDL2) : $(IDL_DIR)/$(TOPIC_IDL) $(IDL_DIR)/$(TOPIC_IDL2)
	$(IDLPP) $(IDLPPFLAGS) $(IDL_DIR)/$(TOPIC_IDL)
	$(IDLPP) $(IDLPPFLAGS) $(IDL_DIR)/$(TOPIC_IDL2)

$(TARGET): $(OBJS)
	$(LD_SO) -L$(SPLICE_LIBRARY_PATH) $(LD_FLAGS) $(OBJS) $(LD_LIBS) -o $(TARGET)
	-mkdir -p SACPP/include
	-mkdir -p SACPP/lib
	cp *.h SACPP/include
	cp $(TARGET) SACPP/lib


clean:
	-rm $(TARGET) $(OBJS) $(IDLPP_IDL) $(IDLPP_CPP) $(IDLPP_HDR)

