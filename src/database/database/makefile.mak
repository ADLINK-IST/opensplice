#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_DATABASE)

ifneq (,$(OSPL_OUTER_HOME))
EXT_CODE_DIR = $(OSPL_OUTER_HOME)/src/database/database/code
CODE_DIR = $(OSPL_HOME)/src/database/database/code
C_FILES = $(notdir $(wildcard $(CODE_DIR)/*.c))
C_FILES += $(notdir $(wildcard $(EXT_CODE_DIR)/*.c))
endif

include		$(OSPL_HOME)/setup/makefiles/target.mak

ifneq (,$(OSPL_OUTER_HOME))
vpath %.c $(EXT_CODE_DIR)
vpath %.h $(EXT_CODE_DIR)
endif

ifneq (,$(ALT_CFLAGS_OPT))
   CFLAGS_OPT=$(ALT_CFLAGS_OPT)
endif

CPPFLAGS	+= -DOSPL_BUILD_DB
ifneq (,$(OSPL_OUTER_HOME))
CPPFLAGS	+= -DUSE_ADV_MEM_MNG
endif

CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS		+= -I$(OSPL_HOME)/src/utilities/include
CINCS		+= -I$(OSPL_HOME)/src/utilities/code
ifneq (,$(OSPL_OUTER_HOME))
CINCS		+= -I$(EXT_CODE_DIR)
endif

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_OS) -l$(DDS_UTIL)

-include $(DEPENDENCIES)
