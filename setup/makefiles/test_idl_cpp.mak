# default values for directory and idl-files to process
IDLPP		:= $(WINCMD) idlpp
IDLPPFLAGS	:= -S -l cpp -I$(OSPL_HOME)/etc/idl -I$(OSPL_HOME)/etc/idl -I$(OSPL_HOME)/etc/idlpp
vpath %.idl     $(IDL_DIR)

# idlpp output
IDLPP_HDR   = $(IDL_FILES:%.idl=ccpp_%.h) $(IDL_FILES:%.idl=%Dcps_impl.h) $(IDL_FILES:%.idl=%SplDcps.h)
IDLPP_CPP   = $(IDL_FILES:%.idl=%SplDcps.cpp) $(IDL_FILES:%.idl=%Dcps_impl.cpp)
IDLPP_IDL   = $(IDL_FILES:%.idl=%Dcps.idl)
IDLPP_OBJ   = $(IDLPP_CPP:%.cpp=%$(OBJ_POSTFIX))

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H       = $(IDLPP_HDR)
IDL_C       = $(IDLPP_CPP)
IDL_O       = $(IDLPP_OBJ)
