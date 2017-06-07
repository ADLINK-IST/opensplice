#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_DCPSSAC)

# Input IDL files.
IDL_DIR     := $(OSPL_HOME)/etc/idl
vpath %.idl $(IDL_DIR)
TOPIC_IDL   := dds_dcps_builtintopics.idl dds_builtinTopics.idl dds_namedQosTypes.idl

# idlpp compiler settings.
IDLPP       = $(WINCMD) idlpp 
IDL_INC_FLAGS= -I$(IDL_DIR)
IDLPPFLAGS  := $(IDL_INC_FLAGS) -N -l c -S

# Finetune idlpp compiler settings when running in Windows.
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
   # Determine import/export macro and include file.
   IDLPPFLAGS += -P SAC_BUILTIN,dds_sac_if.h
endif


# idlpp output
IDL_C   = $(TOPIC_IDL:%.idl=%SacDcps.c) $(TOPIC_IDL:%.idl=%SplDcps.c)
IDL_H   = $(TOPIC_IDL:%.idl=%.h) $(TOPIC_IDL:%.idl=%Dcps.h) $(TOPIC_IDL:%.idl=%SacDcps.h) $(TOPIC_IDL:%.idl=%SplDcps.h)
IDL_O   = $(IDL_C:%.c=%$(OBJ_POSTFIX))

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS += -DOSPL_BUILD_DCPSSAC
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS    += -I../../../common/include
CINCS    += -I$(OSPL_HOME)/src/utilities/include
CINCS    += -I$(OSPL_HOME)/src/database/serialization/include
CINCS    += -I$(OSPL_HOME)/src/database/database/include
CINCS    += -I$(OSPL_HOME)/src/kernel/include
CINCS    += -I$(OSPL_HOME)/src/user/include
CINCS    += -I$(OSPL_HOME)/src/configuration/config/include
CINCS    += -I$(OSPL_HOME)/src/api/dcps/common/include

LDFLAGS   += $(SHLDFLAGS)
LDLIBS    += $(SHLDLIBS)
LDLIBS    += -l$(DDS_CORE)

# Generate the C# interfaces from the IDL descriptions.
%SacDcps.c %SplDcps.c %.h %Dcps.h %SacDcps.h %SplDcps.h: %.idl
	$(IDLPP) $(IDLPPFLAGS) $<

-include $(DEPENDENCIES)
