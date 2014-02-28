# default values for directory and idl-files to process
ifeq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
  ifdef IDL_DIR
    IDL_INC_FLAGS = -I$(IDL_DIR)
  endif
  IDL_INC_FLAGS += -I$(OSPL_HOME)/etc/idl
else
  ifdef IDL_DIR
    TMP_IDL_DIR_INC_FLAG  =-I'$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(IDL_DIR))'
  endif
  TMP_IDL_CCPP_INC_FLAG +=-I'$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_HOME)/etc/idl)'
  IDL_INC_FLAGS = $(TMP_IDL_DIR_INC_FLAG) $(TMP_IDL_CCPP_INC_FLAG)
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

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H		= $(IDLPP_HDR)
IDL_C		= $(IDLPP_CPP)
IDL_O		= $(IDLPP_OBJ)
