# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_UTIL)

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_UTIL
CFLAGS          += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS         += $(SHLDFLAGS)
LDLIBS          += $(SHLDLIBS)

INCLUDE         += -I$(OSPL_HOME)/src/database/database/include

LDLIBS		+= -l$(DDS_OS)

-include $(DEPENDENCIES)
