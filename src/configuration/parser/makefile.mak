#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_CONFPARSER)
EXTRACTED_LIB = libddscore

include	$(OSPL_HOME)/setup/makefiles/target.mak

ifneq (,$(ALT_CFLAGS_OPT))
   CFLAGS_OPT=$(ALT_CFLAGS_OPT)
endif

CPPFLAGS	+= -DOSPL_BUILD_CONFPRS
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/include
CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/configuration/config/include
CINCS		+= -I$(OSPL_HOME)/src/utilities/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_CONF) -l$(DDS_UTIL) -l$(DDS_DATABASE) -l$(DDS_OS)

-include $(DEPENDENCIES)
