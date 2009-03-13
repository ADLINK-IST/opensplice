#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB		:= $(DDS_CONF)

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_CONFFW
CFLAGS          += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS         += $(SHLDFLAGS)
LDLIBS          += $(SHLDLIBS)

INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include

LDLIBS		+= -l$(DDS_DATABASE) -l$(DDS_OS)

-include $(DEPENDENCIES)
