TARGET_SLIB	:= $(DDS_QOSPROVIDER_STATIC)

# Determine import/export macro and include file.


# Explicitly point to an alternative location for all required source files.
CODE_DIR	:= ../../../common/code
QP_CODE	        := $(notdir $(wildcard ../../code/*.c))
C_FILES 	:= $(notdir $(wildcard $(CODE_DIR)/*.c)) 

# Input IDL files.
IDL_DIR     := $(OSPL_HOME)/etc/idl
vpath %.idl $(IDL_DIR)
TOPIC_IDL   := dds_namedQosTypes.idl dds_dcps_builtintopics.idl dds_builtinTopics.idl

# idlpp compiler settings.
IDLPP       = $(WINCMD) idlpp 
IDL_INC_FLAGS= -I$(IDL_DIR)
IDLPPFLAGS  := $(IDL_INC_FLAGS) -l c -S -m SPLLOAD -m SPLTYPE

# idlpp output
IDL_C   = $(TOPIC_IDL:%.idl=%SplLoad.c)

.PRECIOUS: $(IDL_C)


# Include the actual building rules.
include	     $(OSPL_HOME)/setup/makefiles/target.mak

# Fine tune the compiler flags.
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS) -DOSPL_BUILD_DCPSCOMMON_STATIC
CINCS    += -I$(OSPL_HOME)/src/api/dcps/common/include
CINCS    += -I$(OSPL_HOME)/src/database/database/include
CINCS    += -I$(OSPL_HOME)/src/kernel/include
CINCS    += -I$(OSPL_HOME)/src/user/include
CINCS    += -I$(OSPL_HOME)/src/configuration/config/include
CINCS    += -I$(OSPL_HOME)/src/configuration/parser/include
CINCS    += -I$(OSPL_HOME)/src/utilities/include

CXXFLAGS += $(SHCFLAGS) $(MTCFLAGS)

# Fine tune the Linker flags.
LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS   += -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_USER) -l$(DDS_UTIL) -l$(DDS_CONF) -l$(DDS_CONFPARSER)

$(DEPENDENCIES): $(IDL_C)

# Generate the SplLoad interfaces from the IDL descriptions.
%SplLoad.c: %.idl
	$(IDLPP) $(IDLPPFLAGS) $<

-include $(DEPENDENCIES)
