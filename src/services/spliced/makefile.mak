TARGET_DLIB	:= spliced

IDLPP = $(WINCMD) idlpp
IDL_FILE = dds_builtInTypes.idl
IDL_DIR = $(OSPL_HOME)/etc/idl/
IDL_FILES = $(IDL_DIR)$(IDL_FILE)

include	$(OSPL_HOME)/setup/makefiles/test_idl_c.mak

IDL_C		   = $(IDL_FILE:%.idl=%_register.c)
IDL_H		   = $(IDL_FILE:%.idl=%_register.h)
IDLPPTYPES  = DDS::Time_t,DDS::Duration_t
IDLPPFLAGS := -S -l c

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS += -DOSPL_BUILD_SPLICED

CFLAGS  += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS +=  -l$(DDS_CORE)  

CINCS	+= -I$(OSPL_HOME)/src/database/database/include
CINCS	+= -I$(OSPL_HOME)/src/database/serialization/include
CINCS	+= -I$(OSPL_HOME)/src/kernel/include
CINCS	+= -I$(OSPL_HOME)/src/user/include
CINCS	+= -I$(OSPL_HOME)/src/utilities/include
CINCS   += -I$(OSPL_HOME)/src/configuration/config/include

ifneq (,$(findstring int5,$(SPLICE_TARGET)))
CFLAGS += -DCOMPILE_ENTRYPOINT_AS_MAIN
endif

.PHONY: make_idl_preprocessor

$(IDL_FILES): make_idl_preprocessor

make_idl_preprocessor:
	cd $(OSPL_HOME)/src/cpp; make
	cd $(OSPL_HOME)/src/tools/cppgen; make
	cd $(OSPL_HOME)/src/tools/idlpp; make

-include $(DEPENDENCIES)
