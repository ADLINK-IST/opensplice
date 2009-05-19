# pre-existing IDL files
TOPIC_IDL := dds_dcps_builtintopics.idl
DCPS_API_IDL := dds_dcps.idl

# Only valid for Unix/Linux type systems.
OBJ_POSTFIX := .o 

.PRECIOUS: %SplDcps.cpp %Dcps_impl.cpp

# This determines what will be processed 

# OpenSplice Preprocessor (idlpp) output
IDLPP_OBJ        = $(TOPIC_IDL:%.idl=%Dcps_impl.o) $(TOPIC_IDL:%.idl=%SplDcps.o)
IDLPP_SRC        = $(IDLPP_OBJ:%.o=%.cpp)
IDLPP_IDL        = $(TOPIC_IDL:%.idl=%Dcps.idl)
IDLPP_HDR        = $(IDLPP_OBJ:%.o=%.h) ccpp_$(TOPIC_IDL:%.idl=%.h)

# API Classes.
API_SRC          = $(notdir $(wildcard *.cpp))
API_OBJ          = $(API_SRC:%.cpp=%.o)

# All objects
OBJS = $(IDLPP_OBJ) $(API_OBJ)

# library target name
TARGET_DLIB := dcpssacpp
DLIB_PREFIX := lib
DLIB_POSTFIX := .so

TARGET = $(DLIB_PREFIX)$(TARGET_DLIB)$(DLIB_POSTFIX)

CXXINCS += -I./
CXXINCS += -I$(OSPL_HOME)/include
CXXINCS += -I$(OSPL_HOME)/include/sys
CXXINCS += -I$(OSPL_HOME)/include/dcps/C++/SACPP

# compiler and compiler flags (Only valid for gcc-compilers)
CXX := CC
CXXFLAGS := -g -KPIC -mt -xO4
CPPFLAGS = $(CXXINCS)

# linker and linker flags (Only valid for gcc-linkers)
LD_SO := $(CXX)
SPLICE_LIBRARY_PATH := $(OSPL_HOME)/lib
LD_FLAGS := -G -mt -R -xildoff
LD_LIBS  := -lrt -ldl -lpthread -lnsl -ldcpsgapi -lgen -lposix4 -lX11 -lXt -lXm

# SPLICE IDL preprocessor and preprocessor flags
IDLPP := idlpp
IDLPPFLAGS := -P SACPP_API -S -l cpp

#Dependencies

all : $(TARGET)

#generic rules for IDL preprocessing

$(IDLPP_IDL) $(IDLPP_CPP) $(IDLPP_HDR) : $(TOPIC_IDL)
	$(IDLPP) $(IDLPPFLAGS) $<

$(TARGET): $(OBJS) 
	$(LD_SO) -L$(SPLICE_LIBRARY_PATH) $(LD_FLAGS) $(OBJS) $(LD_LIBS) -o $(TARGET)
	-mkdir -p SACPP/include
	-mkdir -p SACPP/lib
	cp $(TOPIC_IDL) $(DCPS_API_IDL) $(IDLPP_HDR) SACPP/include
	cp $(TARGET) SACPP/lib
	

clean:
	-rm $(TARGET) $(OBJS) $(IDLPP_IDL) $(IDLPP_CPP) $(IDLPP_HDR)
	
