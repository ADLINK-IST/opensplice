# default values for directory and idl-files to process
IDL_DIR     ?= ../../code
IDL_FILES   ?= $(notdir $(wildcard $(IDL_DIR)/*.idl))
vpath %.idl     $(IDL_DIR)

# idlpp compiler settings.
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

IDLPP       = $(WINCMD) idlpp
IDLPPFLAGS  = $(IDL_INC_FLAGS) -l isocpp2 -S
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
ifdef DECL_PREFIX
IDLPPFLAGS  += -P$(DECL_PREFIX),$(DECL_INCLUDE)
endif
endif

# idlpp output
IDLPP_HDR   = $(IDL_FILES:%.idl=%.h) $(IDL_FILES:%.idl=%_DCPS.hpp) $(IDL_FILES:%.idl=%SplDcps.h)   
IDLPP_CPP   = $(IDL_FILES:%.idl=%.cpp) $(IDL_FILES:%.idl=%SplDcps.cpp)
IDLPP_IDL   = $(IDL_FILES:%.idl=%Dcps.idl)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H       = $(IDLPP_HDR)
IDL_C       = $(IDLPP_CPP)
IDL_O       = $(IDLPP_OBJ)
