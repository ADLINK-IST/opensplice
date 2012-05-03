#
# included by bld/$(SPLICE_HOST)/makefile


TARGET_EXEC	:= spliced

IDLPP = $(WINCMD) idlpp
IDL_FILE = dds_builtInTypes.idl
IDL_DIR = $(OSPL_HOME)/etc/idl/
IDL_FILES = $(IDL_DIR)$(IDL_FILE)

include	$(OSPL_HOME)/setup/makefiles/test_idl_c.mak

IDL_C             = $(IDL_FILE:%.idl=%_register.c)
IDL_H             = $(IDL_FILE:%.idl=%_register.h)
IDLPPTYPES  = DDS::Time_t,DDS::Duration_t
IDLPPFLAGS := -S -l c

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += -l$(DDS_USER) -l$(DDS_SERIALIZATION) -l$(DDS_CONF) -l$(DDS_CONFPARSER)
LDLIBS += -l$(DDS_UTIL) -l$(DDS_KERNEL) -l$(DDS_DATABASE) -l$(DDS_OS)

CINCS	+= -I$(OSPL_HOME)/src/database/database/include
CINCS	+= -I$(OSPL_HOME)/src/database/serialization/include
CINCS	+= -I$(OSPL_HOME)/src/kernel/include
CINCS	+= -I$(OSPL_HOME)/src/user/include

.PHONY: make_idl_preprocessor

$(IDL_FILES): make_idl_preprocessor

make_idl_preprocessor:
	cd $(OSPL_HOME)/src/cpp; make
	cd $(OSPL_HOME)/src/tools/cppgen; make
	cd $(OSPL_HOME)/src/tools/idlpp; make

-include $(DEPENDENCIES)
## seems TARGET_DEP is not defined anymore
## -include $(TARGET_DEP)
