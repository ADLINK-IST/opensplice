# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB		:= $(DDS_SERIALIZATION)

include		$(OSPL_HOME)/setup/makefiles/target.mak

ifneq ($(strip $(SPECIAL)),)
SD_CPPFLAG := -DSPLICE_HOST_$(SPECIAL)
endif

# Suppression of macro's (QAC)

CPFLAGS         += -macros ../../sd_macros.qac

CPPFLAGS	+= -DOSPL_BUILD_SER
CPPFLAGS        += $(SD_CPPFLAG)
CFLAGS		+= $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS		+= $(SHLDFLAGS)
LDLIBS		+= $(SHLDLIBS)
LDLIBS		+= -l$(DDS_OS) -l$(DDS_DATABASE)

INCLUDE         += -I$(OSPL_HOME)/src/database/database/include
INCLUDE         += -I$(OSPL_HOME)/src/database/database/code

-include $(DEPENDENCIES)
