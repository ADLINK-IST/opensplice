#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_DATABASE)
EXTRACTED_LIB = libddscore

include		$(OSPL_HOME)/setup/makefiles/target.mak

ifneq (,$(ALT_CFLAGS_OPT))
   CFLAGS_OPT=$(ALT_CFLAGS_OPT)
endif

CPPFLAGS	+= -DOSPL_BUILD_DB
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS		+= -I$(OSPL_HOME)/src/utilities/code

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_OS) -l$(DDS_UTIL)

-include $(DEPENDENCIES)
