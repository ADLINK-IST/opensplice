ifeq ($(SPLICE_SA_ORB), DDS_Eorb_3_0_SA)
    ORB_SA_MK_INCLUDE_NAME	= Eorb
    ORB_SA_INCLUDE	     	= -I$(EORBHOME)/include
    ORB_SA_LDLIBS	        = -L$(EORBHOME)/lib/$(EORBENV)
    ORB_SA_IDL_COMPILER 	= idlcpp
    ORB_SA_COMPILER     	= idlcpp #only needed for compiling the corba-C++ testcases
    ORB_SA_IDL_FLAGS		= -dds
    ORB_SA_CPP_FLAGS  		= -DEORB_BUILD_DDS
    ORB_SA_SELECT_FLAGS 	= -D$(SPLICE_SA_ORB)
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
    ORB_SA_CXX_FLAGS        = -import_export=$(DECL_PREFIX)
endif

# various rules to define ORB specific source and object files
    ORB_TOP_OBJ      = $(TOPIC_IDL:%.idl=%$(OBJ_POSTFIX))
    ORB_TOP_SRC      = $(TOPIC_IDL:%.idl=%.cpp)
    ORB_TOP_HDR      = $(ORB_TOP_SRC:%.cpp=%.h)

    ORB_API_OBJ      = $(DCPS_API_IDL:%.idl=%$(OBJ_POSTFIX))
    ORB_API_SRC      = $(DCPS_API_IDL:%.idl=%.cpp)
    ORB_API_HDR      = $(ORB_API_SRC:%.cpp=%.h)

    IDLPP_ORB_OBJ    = $(TOPIC_IDL:%.idl=%Dcps$(OBJ_POSTFIX))
    IDLPP_ORB_SRC    = $(TOPIC_IDL:%.idl=%Dcps.cpp)
    IDLPP_ORB_HDR    = $(IDLPP_ORB_SRC:%.cpp=%.h)

    ORB_DLRL_API_OBJ = $(DLRL_API_IDL:%.idl=%$(OBJ_POSTFIX))
    ORB_DLRL_API_SRC = $(DLRL_API_IDL:%.idl=%.cpp)
    ORB_DLRL_API_HDR = $(ORB_DLRL_API_SRC:%.cpp=%.h)
endif
