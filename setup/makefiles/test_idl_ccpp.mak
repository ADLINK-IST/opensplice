# default values for directory and idl-files to process
DCPS_IDL   = $(TOPIC_IDL:%.idl=%Dcps.idl)
IDL_FILES  = $(TOPIC_IDL) $(DCPS_IDL)

ifeq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
IDL_INC_FLAGS   = -I$(IDL_DIR) -I$(OSPL_HOME)/etc/idl
else
TMP_IDL_DIR_INC_FLAG  =$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(IDL_DIR))
TMP_IDL_CCPP_INC_FLAG =$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_HOME)/etc/idl)
IDL_INC_FLAGS   = -I'$(TMP_IDL_DIR_INC_FLAG)' -I'$(TMP_IDL_CCPP_INC_FLAG)'
endif
vpath %.idl	$(IDL_DIR)

# Import all ORB specific data.
include         $(OSPL_HOME)/setup/makefiles/orbdeps.mak

# idlpp compiler settings.
IDLPP       := idlpp
IDLPPFLAGS  := $(IDL_INC_FLAGS) -l cpp -C
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
ifdef DECL_PREFIX
IDLPPFLAGS  += -P$(DECL_PREFIX),$(DECL_INCLUDE)
endif
endif

# idlpp output
IDLPP_HDR   = $(TOPIC_IDL:%.idl=ccpp_%.h) $(TOPIC_IDL:%.idl=%Dcps_impl.h) $(TOPIC_IDL:%.idl=%SplDcps.h)
IDLPP_CPP   = $(TOPIC_IDL:%.idl=%SplDcps.cpp) $(TOPIC_IDL:%.idl=%Dcps_impl.cpp)
IDLPP_IDL   = $(TOPIC_IDL:%.idl=%Dcps.idl)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

# ORB IDL compiler output
IDLCC_H     = $(IDL_FILES:%.idl=%C.h)
IDLCC_CPP   = $(IDL_FILES:%.idl=%C.cpp)
IDLCC_OBJ   = $(IDL_FILES:%.idl=%C$(OBJ_POSTFIX))

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H       = $(IDLPP_HDR) $(IDLCC_H)
IDL_C       = $(IDLPP_CPP) $(IDLCC_CPP)
IDL_O       = $(IDLPP_OBJ) $(IDLCC_OBJ)
