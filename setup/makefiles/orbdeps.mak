# This makefile defines the make rules for the used ORB (Tao 1.4.1 openfusion, ACE-TAO 1.4.1, Mico 2.3.11)

#ORB dependent settings:

IF_OSPLENV_IS_WIN32:=$(findstring win32, $(SPLICE_TARGET))
IF_OSPLMODE_IS_DEV:=$(findstring dev, $(SPLICE_TARGET))
IS_WINDOWS_DEBUG:=$(and $(IF_OSPLENV_IS_WIN32), $(IF_OSPLMODE_IS_DEV))

ifeq ($(SPLICE_ORB), DDS_ACE_TAO_5_6_6)
    ORB_MK_INCLUDE_NAME = tao15-OF
    ORB_INCLUDE	     = "-I$(TAO_ROOT)" -I"$(ACE_ROOT)"
    ORB_LDLIBS	     = "-L$(ACE_ROOT)/lib" -lACE -lTAO -lTAO_PortableServer -lTAO_AnyTypeCode
    ORB_IDL_COMPILER = tao_idl
    ORB_COMPILER     = tao_idl #only needed for compiling the corba-C++ testcases
    ORB_IDL_FLAGS    = -Sp -Sd -si S.i -ci C.i -I"$(OSPL_HOME)/etc/idl"
    ORB_CPP_FLAGS    = -DACE_HAS_EXCEPTIONS
    ORB_SELECT_FLAGS = -D$(SPLICE_ORB)
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
    ORB_CXX_FLAGS    = -Wb,export_macro=$(DECL_PREFIX) -Wb,export_include=$(DECL_INCLUDE)
endif

# various rules to define ORB specific source and object files
    ORB_TOP_OBJ      = $(TOPIC_IDL:%.idl=%C$(OBJ_POSTFIX)) $(TOPIC_IDL2:%.idl=%C$(OBJ_POSTFIX))
    ORB_TOP_SRC      = $(TOPIC_IDL:%.idl=%C.cpp) $(TOPIC_IDL:%.idl=%S.cpp) $(TOPIC_IDL2:%.idl=%C.cpp) $(TOPIC_IDL2:%.idl=%S.cpp)
    ORB_TOP_HDR      = $(ORB_TOP_SRC:%.cpp=%.h) $(ORB_TOP_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(ORB_TOP_SRC2:%.cpp=%.h) $(ORB_TOP_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    ORB_API_OBJ      = $(DCPS_API_IDL:%.idl=%C$(OBJ_POSTFIX)) $(INT_IDL:%.idl=%C$(OBJ_POSTFIX))
    ORB_API_SRC      = $(DCPS_API_IDL:%.idl=%C.cpp) $(DCPS_API_IDL:%.idl=%S.cpp) $(INT_IDL:%.idl=%C.cpp) $(INT_IDL:%.idl=%S.cpp)
    ORB_API_HDR      = $(ORB_API_SRC:%.cpp=%.h) $(ORB_API_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(ORB_API_SRC2:%.cpp=%.h) $(ORB_API_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    IDLPP_ORB_OBJ    = $(TOPIC_IDL:%.idl=%DcpsC$(OBJ_POSTFIX)) $(TOPIC_IDL2:%.idl=%DcpsC$(OBJ_POSTFIX))
    IDLPP_ORB_SRC    = $(TOPIC_IDL:%.idl=%DcpsC.cpp) $(TOPIC_IDL:%.idl=%DcpsS.cpp) $(TOPIC_IDL2:%.idl=%DcpsC.cpp) $(TOPIC_IDL2:%.idl=%DcpsS.cpp)
    IDLPP_ORB_HDR    = $(IDLPP_ORB_SRC:%.cpp=%.h) $(IDLPP_ORB_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(IDLPP_ORB_SRC2:%.cpp=%.h) $(IDLPP_ORB_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    #TODO determine if correct
    ORB_DLRL_API_OBJ = $(DLRL_API_IDL:%.idl=%C$(OBJ_POSTFIX))
    ORB_DLRL_API_SRC = $(DLRL_API_IDL:%.idl=%C.cpp) $(DLRL_API_IDL:%.idl=%S.cpp)
    ORB_DLRL_API_HDR = $(ORB_DLRL_API_SRC:%.cpp=%.h) $(ORB_DLRL_API_SRC:%.cpp=%.inl)
endif
ifeq ($(SPLICE_ORB), DDS_OpenFusion_1_6_1)
    ORB_MK_INCLUDE_NAME = tao15-OF
    ORB_INCLUDE	     = "-I$(TAO_ROOT)/include"
ifdef IS_WINDOWS_DEBUG
    ORB_LDLIBS	     = "-L$(TAO_ROOT)/lib" -lACEd -lTAOd -lTAO_PortableServerd -lTAO_AnyTypeCoded
else
    ORB_LDLIBS	     = "-L$(TAO_ROOT)/lib" -lACE -lTAO -lTAO_PortableServer -lTAO_AnyTypeCode
endif
    ORB_IDL_COMPILER = tao_idl
    ORB_COMPILER     = tao_idl #only needed for compiling the corba-C++ testcases
    ORB_IDL_FLAGS    = -Sp -Sd -si S.i -ci C.i -I"$(OSPL_HOME)/etc/idl"
    ORB_IDL_OUTPUT   = -o
    ORB_CPP_FLAGS    = -DACE_HAS_EXCEPTIONS
    ORB_SELECT_FLAGS = -D$(SPLICE_ORB)
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
    ORB_CXX_FLAGS    = -Wb,export_macro=$(DECL_PREFIX) -Wb,export_include=$(DECL_INCLUDE)
endif

# various rules to define ORB specific source and object files
    ORB_TOP_OBJ      = $(TOPIC_IDL:%.idl=%C$(OBJ_POSTFIX)) $(TOPIC_IDL2:%.idl=%C$(OBJ_POSTFIX))
    ORB_TOP_SRC      = $(TOPIC_IDL:%.idl=%C.cpp) $(TOPIC_IDL:%.idl=%S.cpp) $(TOPIC_IDL2:%.idl=%C.cpp) $(TOPIC_IDL2:%.idl=%S.cpp)
    ORB_TOP_HDR      = $(ORB_TOP_SRC:%.cpp=%.h) $(ORB_TOP_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(ORB_TOP_SRC2:%.cpp=%.h) $(ORB_TOP_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    ORB_API_OBJ      = $(DCPS_API_IDL:%.idl=%C$(OBJ_POSTFIX)) $(INT_IDL:%.idl=%C$(OBJ_POSTFIX))
    ORB_API_SRC      = $(DCPS_API_IDL:%.idl=%C.cpp) $(DCPS_API_IDL:%.idl=%S.cpp) $(INT_IDL:%.idl=%C.cpp) $(INT_IDL:%.idl=%S.cpp)
    ORB_API_HDR      = $(ORB_API_SRC:%.cpp=%.h) $(ORB_API_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(ORB_API_SRC2:%.cpp=%.h) $(ORB_API_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    IDLPP_ORB_OBJ    = $(TOPIC_IDL:%.idl=%DcpsC$(OBJ_POSTFIX)) $(TOPIC_IDL2:%.idl=%DcpsC$(OBJ_POSTFIX))
    IDLPP_ORB_SRC    = $(TOPIC_IDL:%.idl=%DcpsC.cpp) $(TOPIC_IDL:%.idl=%DcpsS.cpp) $(TOPIC_IDL2:%.idl=%DcpsC.cpp) $(TOPIC_IDL2:%.idl=%DcpsS.cpp)
    IDLPP_ORB_HDR    = $(IDLPP_ORB_SRC:%.cpp=%.h) $(IDLPP_ORB_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(IDLPP_ORB_SRC2:%.cpp=%.h) $(IDLPP_ORB_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    #TODO determine if correct
    ORB_DLRL_API_OBJ = $(DLRL_API_IDL:%.idl=%C$(OBJ_POSTFIX))
    ORB_DLRL_API_SRC = $(DLRL_API_IDL:%.idl=%C.cpp) $(DLRL_API_IDL:%.idl=%S.cpp)
    ORB_DLRL_API_HDR = $(ORB_DLRL_API_SRC:%.cpp=%.h) $(ORB_DLRL_API_SRC:%.cpp=%.inl)
endif
ifeq ($(SPLICE_ORB), DDS_OpenFusion_1_5_1)
    ORB_MK_INCLUDE_NAME = tao15-OF
    ORB_INCLUDE	     = "-I$(TAO_ROOT)/include"
ifdef IS_WINDOWS_DEBUG
    ORB_LDLIBS	     = "-L$(TAO_ROOT)/lib" -lACEd -lTAOd -lTAO_PortableServerd -lTAO_AnyTypeCoded
else
    ORB_LDLIBS	     = "-L$(TAO_ROOT)/lib" -lACE -lTAO -lTAO_PortableServer -lTAO_AnyTypeCode
endif
    ORB_IDL_COMPILER = tao_idl
    ORB_COMPILER     = tao_idl #only needed for compiling the corba-C++ testcases
    ORB_IDL_FLAGS    = -Sc -Sp -Sd -si S.i -ci C.i -I"$(OSPL_HOME)/etc/idl"
    ORB_IDL_OUTPUT   = -o
    ORB_CPP_FLAGS    = -DACE_HAS_EXCEPTIONS
    ORB_SELECT_FLAGS = -D$(SPLICE_ORB)
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
    ORB_CXX_FLAGS    = -Wb,export_macro=$(DECL_PREFIX) -Wb,export_include=$(DECL_INCLUDE)
endif

# various rules to define ORB specific source and object files
    ORB_TOP_OBJ      = $(TOPIC_IDL:%.idl=%C$(OBJ_POSTFIX)) $(TOPIC_IDL2:%.idl=%C$(OBJ_POSTFIX))
    ORB_TOP_SRC      = $(TOPIC_IDL:%.idl=%C.cpp) $(TOPIC_IDL:%.idl=%S.cpp) $(TOPIC_IDL2:%.idl=%C.cpp) $(TOPIC_IDL2:%.idl=%S.cpp)
    ORB_TOP_HDR      = $(ORB_TOP_SRC:%.cpp=%.h) $(ORB_TOP_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(ORB_TOP_SRC2:%.cpp=%.h) $(ORB_TOP_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    ORB_API_OBJ      = $(DCPS_API_IDL:%.idl=%C$(OBJ_POSTFIX)) $(INT_IDL:%.idl=%C$(OBJ_POSTFIX))
    ORB_API_SRC      = $(DCPS_API_IDL:%.idl=%C.cpp) $(DCPS_API_IDL:%.idl=%S.cpp) $(INT_IDL:%.idl=%C.cpp) $(INT_IDL:%.idl=%S.cpp)
    ORB_API_HDR      = $(ORB_API_SRC:%.cpp=%.h) $(ORB_API_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(ORB_API_SRC2:%.cpp=%.h) $(ORB_API_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    IDLPP_ORB_OBJ    = $(TOPIC_IDL:%.idl=%DcpsC$(OBJ_POSTFIX)) $(TOPIC_IDL2:%.idl=%DcpsC$(OBJ_POSTFIX))
    IDLPP_ORB_SRC    = $(TOPIC_IDL:%.idl=%DcpsC.cpp) $(TOPIC_IDL:%.idl=%DcpsS.cpp) $(TOPIC_IDL2:%.idl=%DcpsC.cpp) $(TOPIC_IDL2:%.idl=%DcpsS.cpp)
    IDLPP_ORB_HDR    = $(IDLPP_ORB_SRC:%.cpp=%.h) $(IDLPP_ORB_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(IDLPP_ORB_SRC2:%.cpp=%.h) $(IDLPP_ORB_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    #TODO determine if correct
    ORB_DLRL_API_OBJ = $(DLRL_API_IDL:%.idl=%C$(OBJ_POSTFIX))
    ORB_DLRL_API_SRC = $(DLRL_API_IDL:%.idl=%C.cpp) $(DLRL_API_IDL:%.idl=%S.cpp)
    ORB_DLRL_API_HDR = $(ORB_DLRL_API_SRC:%.cpp=%.h) $(ORB_DLRL_API_SRC:%.cpp=%.inl)
endif
ifeq ($(SPLICE_ORB), DDS_OpenFusion_1_4_1)
    ORB_MK_INCLUDE_NAME = tao14-OF
    ORB_INCLUDE	     = "-I$(TAO_ROOT)/include"
ifdef IS_WINDOWS_DEBUG
    ORB_LDLIBS	     = "-L$(TAO_ROOT)/lib" -lACEd -lTAOd -lTAO_PortableServerd
else
    ORB_LDLIBS	     = "-L$(TAO_ROOT)/lib" -lACE -lTAO -lTAO_PortableServer
endif
    ORB_IDL_COMPILER = tao_idl
    ORB_COMPILER     = tao_idl #only needed for compiling the corba-C++ testcases
    ORB_IDL_FLAGS    = -Sc -Sp -Sd -Sv -I"$(OSPL_HOME)/etc/idl"
    ORB_IDL_OUTPUT   = -o
    ORB_CPP_FLAGS    = -DACE_HAS_EXCEPTIONS
    ORB_SELECT_FLAGS = -D$(SPLICE_ORB)
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
    ORB_CXX_FLAGS    = -Wb,export_macro=$(DECL_PREFIX) -Wb,export_include=$(DECL_INCLUDE)
endif

# various rules to define ORB specific source and object files
    ORB_TOP_OBJ      = $(TOPIC_IDL:%.idl=%C$(OBJ_POSTFIX)) $(TOPIC_IDL2:%.idl=%C$(OBJ_POSTFIX))
    ORB_TOP_SRC      = $(TOPIC_IDL:%.idl=%C.cpp) $(TOPIC_IDL:%.idl=%S.cpp) $(TOPIC_IDL2:%.idl=%C.cpp) $(TOPIC_IDL2:%.idl=%S.cpp)
    ORB_TOP_HDR      = $(ORB_TOP_SRC:%.cpp=%.h) $(ORB_TOP_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(ORB_TOP_SRC2:%.cpp=%.h) $(ORB_TOP_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    ORB_API_OBJ      = $(DCPS_API_IDL:%.idl=%C$(OBJ_POSTFIX)) $(INT_IDL:%.idl=%C$(OBJ_POSTFIX))
    ORB_API_SRC      = $(DCPS_API_IDL:%.idl=%C.cpp) $(DCPS_API_IDL:%.idl=%S.cpp) $(INT_IDL:%.idl=%C.cpp) $(INT_IDL:%.idl=%S.cpp)
    ORB_API_HDR      = $(ORB_API_SRC:%.cpp=%.h) $(ORB_API_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(ORB_API_SRC2:%.cpp=%.h) $(ORB_API_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    IDLPP_ORB_OBJ    = $(TOPIC_IDL:%.idl=%DcpsC$(OBJ_POSTFIX)) $(TOPIC_IDL2:%.idl=%DcpsC$(OBJ_POSTFIX))
    IDLPP_ORB_SRC    = $(TOPIC_IDL:%.idl=%DcpsC.cpp) $(TOPIC_IDL:%.idl=%DcpsS.cpp) $(TOPIC_IDL2:%.idl=%DcpsC.cpp) $(TOPIC_IDL2:%.idl=%DcpsS.cpp)
    IDLPP_ORB_HDR    = $(IDLPP_ORB_SRC:%.cpp=%.h) $(IDLPP_ORB_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(IDLPP_ORB_SRC2:%.cpp=%.h) $(IDLPP_ORB_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    ORB_DLRL_API_OBJ = $(DLRL_API_IDL:%.idl=%C$(OBJ_POSTFIX))
    ORB_DLRL_API_SRC = $(DLRL_API_IDL:%.idl=%C.cpp) $(DLRL_API_IDL:%.idl=%S.cpp)
    ORB_DLRL_API_HDR = $(ORB_DLRL_API_SRC:%.cpp=%.h) $(ORB_DLRL_API_SRC:%.cpp=%$(INLINESRC_POSTFIX))
endif
ifeq ($(SPLICE_ORB), DDS_Mico_2_3_11)
    ORB_MK_INCLUDE_NAME = Mico
    ORB_INCLUDE      = -I$(MICO_ROOT)/include
    ORB_LDLIBS	     = -L$(MICO_ROOT)/lib -lmico2.3.11 -lmicocoss2.3.11 -lmicoaux2.3.11 -lmicoir2.3.11 -lmicox2.3.11
    ORB_IDL_COMPILER = idl
    ORB_COMPILER     = idl #only needed for compiling the corba-C++ testcases
    ORB_IDL_FLAGS    = --c++-suffix=cpp
    ORB_CPP_FLAGS    = # ORB specific macro definitions
    ORB_CXX_FLAGS    = # ORB implied C++ compiler specific flags.
    ORB_SELECT_FLAGS = -D$(SPLICE_ORB)

# various rules to define ORB specific source and object files
    ORB_TOP_OBJ      = $(TOPIC_IDL:%.idl=%$(OBJ_POSTFIX))
    ORB_TOP_SRC      = $(ORB_TOP_OBJ:%=%.cpp)
    ORB_TOP_HDR      = $(ORB_TOP_SRC:%.cpp=%.h)

    ORB_API_OBJ      = $(DCPS_API_IDL:%.idl=%$(OBJ_POSTFIX))
    ORB_API_SRC      = $(ORB_API_OBJ:%$(OBJ_POSTFIX)=%.cpp)
    ORB_API_HDR      = $(ORB_API_SRC:%.cpp=%.h)

    IDLPP_ORB_OBJ    = $(TOPIC_IDL:%.idl=%Dcps$(OBJ_POSTFIX))
    IDLPP_ORB_SRC    = $(IDLPP_ORB_OBJ:%$(OBJ_POSTFIX)=%.cpp)
    IDLPP_ORB_HDR    = $(IDLPP_ORB_OBJ:%$(OBJ_POSTFIX)=%.h)

    ORB_DLRL_API_OBJ = $(DLRL_API_IDL:%.idl=%$(OBJ_POSTFIX))
    ORB_DLRL_API_SRC = $(ORB_DLRL_API_OBJ:%$(OBJ_POSTFIX)=%.cpp)
    ORB_DLRL_API_HDR = $(ORB_DLRL_API_SRC:%.cpp=%.h)
endif
ifeq ($(SPLICE_ORB), DDS_ACE_TAO_1_4_1)
    ORB_MK_INCLUDE_NAME = TAO
    ORB_INCLUDE      = "-I$(ACE_ROOT)" "-I$(TAO_ROOT)"
    ORB_LDLIBS       = "-L$(ACE_ROOT)/ace" -lACE -lTAO -lTAO_PortableServer
    ORB_IDL_COMPILER = tao_idl
    ORB_COMPILER     = tao_idl #only needed for compiling the corba-C++ testcases
    ORB_IDL_FLAGS    = -Sc -Sp -Sd -Sv -I"$(OSPL_HOME)/etc/idl"
    ORB_CPP_FLAGS    = -DACE_HAS_EXCEPTIONS
    ORB_SELECT_FLAGS = -D$(SPLICE_ORB)
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
    ORB_CXX_FLAGS    = -Wb,export_macro=$(DECL_PREFIX) -Wb,export_include=$(DECL_INCLUDE)
endif

# various rules to define ORB specific source and object files
    ORB_TOP_OBJ      = $(TOPIC_IDL:%.idl=%C$(OBJ_POSTFIX)) $(TOPIC_IDL2:%.idl=%C$(OBJ_POSTFIX))
    ORB_TOP_SRC      = $(TOPIC_IDL:%.idl=%C.cpp) $(TOPIC_IDL:%.idl=%S.cpp) $(TOPIC_IDL2:%.idl=%C.cpp) $(TOPIC_IDL2:%.idl=%S.cpp)
    ORB_TOP_HDR      = $(ORB_TOP_SRC:%.cpp=%.h) $(ORB_TOP_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(ORB_TOP_SRC2:%.cpp=%.h) $(ORB_TOP_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    ORB_API_OBJ      = $(DCPS_API_IDL:%.idl=%C$(OBJ_POSTFIX)) $(INT_IDL:%.idl=%C$(OBJ_POSTFIX))
    ORB_API_SRC      = $(DCPS_API_IDL:%.idl=%C.cpp) $(DCPS_API_IDL:%.idl=%S.cpp) $(INT_IDL:%.idl=%C.cpp) $(INT_IDL:%.idl=%S.cpp)
    ORB_API_HDR      = $(ORB_API_SRC:%.cpp=%.h) $(ORB_API_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(ORB_API_SRC2:%.cpp=%.h) $(ORB_API_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    IDLPP_ORB_OBJ    = $(TOPIC_IDL:%.idl=%DcpsC$(OBJ_POSTFIX)) $(TOPIC_IDL2:%.idl=%DcpsC$(OBJ_POSTFIX))
    IDLPP_ORB_SRC    = $(TOPIC_IDL:%.idl=%DcpsC.cpp) $(TOPIC_IDL:%.idl=%DcpsS.cpp) $(TOPIC_IDL2:%.idl=%DcpsC.cpp) $(TOPIC_IDL2:%.idl=%DcpsS.cpp)
    IDLPP_ORB_HDR    = $(IDLPP_ORB_SRC:%.cpp=%.h) $(IDLPP_ORB_SRC:%.cpp=%$(INLINESRC_POSTFIX)) $(IDLPP_ORB_SRC2:%.cpp=%.h) $(IDLPP_ORB_SRC2:%.cpp=%$(INLINESRC_POSTFIX))

    ORB_DLRL_API_OBJ = $(DLRL_API_IDL:%.idl=%C$(OBJ_POSTFIX))
    ORB_DLRL_API_SRC = $(DLRL_API_IDL:%.idl=%C.cpp) $(DLRL_API_IDL:%.idl=%S.cpp)
    ORB_DLRL_API_HDR = $(ORB_DLRL_API_SRC:%.cpp=%.h) $(ORB_DLRL_API_SRC:%.cpp=%$(INLINESRC_POSTFIX))
endif
ifeq ($(SPLICE_ORB), DDS_Eorb_3_0)
    ORB_MK_INCLUDE_NAME = Eorb
    ORB_INCLUDE	     = -I$(EORBHOME)/include
    ORB_LDLIBS	     = -L$(EORBHOME)/lib/$(EORBENV)
    ORB_IDL_COMPILER = idlcpp
    ORB_COMPILER     = idlcpp #only needed for compiling the corba-C++ testcases
    ORB_IDL_FLAGS    = -client_only
    ORB_CPP_FLAGS    = -D$(EORB_PLATFORM_MACRO)
    ORB_CXX_FLAGS    = # ORB implied C++ compiler specific flags.
    ORB_SELECT_FLAGS = -D$(SPLICE_ORB)

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
