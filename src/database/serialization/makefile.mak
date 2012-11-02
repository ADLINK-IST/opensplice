# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB := $(DDS_SERIALIZATION)
EXTRACTED_LIB = libddscore

include $(OSPL_HOME)/setup/makefiles/target.mak

ifneq ($(strip $(SPECIAL)),)
SD_CPPFLAG := -DSPLICE_HOST_$(SPECIAL)
endif

# Suppression of macro's (QAC)
CPFLAGS  += -macros ../../sd_macros.qac

CPPFLAGS	+= -DOSPL_BUILD_SER
CPPFLAGS += $(SD_CPPFLAG)
CFLAGS	+= $(SHCFLAGS) $(MTCFLAGS)
CINCS    += -I$(OSPL_HOME)/src/database/database/include
CINCS    += -I$(OSPL_HOME)/src/database/database/code

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_OS) -l$(DDS_DATABASE)

-include $(DEPENDENCIES)
