#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= dcpsc99

# Input IDL files.
IDL_DIR     := $(OSPL_HOME)/etc/idl
vpath %.idl $(IDL_DIR)
TOPIC_IDL   := dds_dcps_builtintopics.idl dds_builtinTopics.idl dds_namedQosTypes.idl dds_predefTypes.idl dds_IoTData.idl

# idlpp compiler settings.
IDLPP       = $(WINCMD) idlpp
IDL_INC_FLAGS= -I$(IDL_DIR)
IDLPPFLAGS  := $(IDL_INC_FLAGS) -N -l c99 -S

# Finetune idlpp compiler settings when running in Windows.
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
   # Determine import/export macro and include file.
   IDLPPFLAGS += -P C99_BUILTIN,dds_c99_if.h
endif

# Compile only predefTypes generated code, others are already included by linking with SAC library
IDL_C   = dds_predefTypesDcps.c dds_predefTypesSacDcps.c dds_predefTypesSplDcps.c dds_IoTDataDcps.c dds_IoTDataSacDcps.c dds_IoTDataSplDcps.c
IDL_H   = $(TOPIC_IDL:%.idl=%.h) $(TOPIC_IDL:%.idl=%Dcps.h) $(TOPIC_IDL:%.idl=%SacDcps.h) $(TOPIC_IDL:%.idl=%SplDcps.h)
IDL_O   = $(IDL_C:%.c=%$(OBJ_POSTFIX))

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS += -DOSPL_BUILD_DCPSC99
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS    += -I../../../common/include
CINCS    += -I$(OSPL_HOME)/src/utilities/include
CINCS    += -I$(OSPL_HOME)/src/database/serialization/include
CINCS    += -I$(OSPL_HOME)/src/database/database/include
CINCS    += -I$(OSPL_HOME)/src/kernel/include
CINCS    += -I$(OSPL_HOME)/src/user/include
CINCS    += -I$(OSPL_HOME)/src/configuration/config/include
CINCS    += -I$(OSPL_HOME)/src/api/dcps/common/include
CINCS    += -I$(OSPL_HOME)/src/api/dcps/sac/include

LDFLAGS   += $(SHLDFLAGS)
LDLIBS    += $(SHLDLIBS)
LDLIBS    += -l$(DDS_DCPSSAC) -l$(DDS_CORE)

%SacDcps.c %SplDcps.c %.h %Dcps.h %SacDcps.h %SplDcps.h: %.idl
	$(IDLPP) $(IDLPPFLAGS) $<

-include $(DEPENDENCIES)
