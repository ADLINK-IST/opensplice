# default values for directory and idl-files to process
ifeq (,$(findstring win32,$(SPLICE_TARGET)))
  ifdef IDL_DIR
    IDL_INC_FLAGS = -I$(IDL_DIR)
  endif
  IDL_INC_FLAGS += -I$(OSPL_HOME)/etc/idl
  DELIM=:
else
  ifdef IDL_DIR
    TMP_IDL_DIR_INC_FLAG  =-I'$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(IDL_DIR))'
  endif
  TMP_IDL_CCPP_INC_FLAG +=-I'$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_HOME)/etc/idl)'
  IDL_INC_FLAGS = $(TMP_IDL_DIR_INC_FLAG) $(TMP_IDL_CCPP_INC_FLAG)
  DELIM=;
endif

ifdef OSPL_OUTER_HOME
   ifeq (,$(findstring win32,$(SPLICE_TARGET)))
      IDL_INC_FLAGS += -I$(OSPL_OUTER_HOME)/src/api/dlrl/ccpp/idl
   else
      IDL_INC_FLAGS += -I'$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_OUTER_HOME)/src/api/dlrl/ccpp/idl)'
   endif
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
JAR_INC_DIR =$(OSPL_HOME)/jar/$(SPLICE_TARGET)
OUTER_HOME_PATH =`$(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_OUTER_HOME)`

T1=$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(JAR_INC_DIR)/ospldcg.jar)
T2=$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_HOME)/jar/flexlm.jar)
T3=$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_HOME)/jar/EccpressoAll.jar)
JAR_DIR =$(T1)$(DELIM)$(T2)$(DELIM)$(T3)

OSPLDCG =java "-DPTECH_LICENSE_FILE=$(LM_LICENSE_FILE)" "-DOSPL_OUTER_HOME=$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_OUTER_HOME))" -classpath "$(JAR_DIR)" DCG.Control.DCGStarter
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
