#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB		:= $(DDS_DCPSSAJ)

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_DCPSSAJ
CFLAGS          += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS         += $(SHLDFLAGS)
LDLIBS          += $(SHLDLIBS)

INCLUDE		+= -I$(OSPL_HOME)/src/api/dcps/gapi/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/include
INCLUDE		+= -I$(OSPL_HOME)/src/tools/idlpp/include
INCLUDE		+= $(JAVA_INCLUDE)

LDLIBS		+= -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_DCPSGAPI)

-include $(DEPENDENCIES)
