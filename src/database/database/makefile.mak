#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_DATABASE)

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_DB
CFLAGS		+= $(SHCFLAGS) $(MTCFLAGS)

INCLUDE		+= -I$(OSPL_HOME)/src/utilities/code

LDFLAGS		+= $(SHLDFLAGS)
LDLIBS		+= $(SHLDLIBS)
LDLIBS		+= -l$(DDS_OS) -l$(DDS_UTIL)

-include $(DEPENDENCIES)
