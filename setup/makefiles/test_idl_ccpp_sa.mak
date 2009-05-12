# default values for directory and idl-files to process 
IDL_INC_FLAGS   = -I$(IDL_DIR) -I$(OSPL_HOME)/src/api/dcps/ccpp/idl 

ifdef OSPL_OUTER_HOME
IDL_INC_FLAGS  += -I$(OSPL_OUTER_HOME)/src/api/dlrl/ccpp/idl
endif

vpath %.idl	$(IDL_DIR)

# idlpp compiler settings.
IDLPP		:= idlpp 
IDLPPFLAGS	:= $(IDL_INC_FLAGS) -l cpp -S

# idlpp output
IDLPP_HDR   = $(TOPIC_IDL:%.idl=ccpp_%.h) $(TOPIC_IDL:%.idl=%Dcps_impl.h) $(TOPIC_IDL:%.idl=%SplDcps.h) $(TOPIC_IDL:%.idl=%.h) $(TOPIC_IDL:%.idl=%Dcps.h)
IDLPP_CPP   = $(TOPIC_IDL:%.idl=%SplDcps.cpp) $(TOPIC_IDL:%.idl=%Dcps_impl.cpp) $(TOPIC_IDL:%.idl=%.cpp) $(TOPIC_IDL:%.idl=%Dcps.cpp)
IDLPP_IDL   = $(TOPIC_IDL:%.idl=%Dcps.idl)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

# ospldcg compiler settings.
OSPLDCG      := ospldcg 
OSPLDCGFLAGS +=  -l SACPP $(IDL_INC_FLAGS)

# ospldcg output
OSPLDCG_PREFIX   = $(addprefix ccpp_,$(DLRL_XML))
OSPLDCG_ORB_OBJ  = $(DLRL_XML:%.xml=%Object$(OBJ_POSTFIX)) $(DLRL_XML:%.xml=%ObjectDlrl$(OBJ_POSTFIX)) $(OSPLDCG_PREFIX:%.xml=%Object_abstract$(OBJ_POSTFIX)) $(OSPLDCG_PREFIX:%.xml=%Object_impl$(OBJ_POSTFIX)) $(OSPLDCG_PREFIX:%.xml=%ObjectDlrl_impl$(OBJ_POSTFIX))
OSPLDCG_ORB_SRC  = $(OSPLDCG_ORB_OBJ:%$(OBJ_POSTFIX)=%.cpp)
OSPLDCG_ORB_HDR  = $(OSPLDCG_ORB_SRC:%.cpp=%.h) $(OSPLDCG_PREFIX:%.xml=%Object.h)

# This determines what/how it will be processed 
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H		= $(IDLPP_HDR) $(OSPLDCG_ORB_HDR)
IDL_C		= $(IDLPP_CPP) $(OSPLDCG_ORB_SRC)
IDL_O		= $(IDLPP_OBJ) $(OSPLDCG_ORB_OBJ)