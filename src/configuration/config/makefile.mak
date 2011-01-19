#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB := $(DDS_CONF)
EXTRACTED_LIB = libddscore

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_CONFFW
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS    += -I$(OSPL_HOME)/src/database/database/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_DATABASE) -l$(DDS_OS)

-include $(DEPENDENCIES)
