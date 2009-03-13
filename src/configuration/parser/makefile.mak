#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB		:= $(DDS_CONFPARSER)

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_CONFPRS
CFLAGS          += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS         += $(SHLDFLAGS)
LDLIBS          += $(SHLDLIBS)

INCLUDE		+= -I$(OSPL_HOME)/src/abstraction/os/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/configuration/config/include
INCLUDE		+= -I$(OSPL_HOME)/src/utilities/include

LDLIBS		+= -l$(DDS_CONF) -l$(DDS_UTIL) -l$(DDS_DATABASE) -l$(DDS_OS)

-include $(DEPENDENCIES)
