#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB		:= $(DDS_DCPSGAPI)

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_DCPSGAPI
CFLAGS          += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS         += $(SHLDFLAGS)
LDLIBS          += $(SHLDLIBS)

ifeq ($(SPECIAL),purify)
CPPFLAGS         += -D_PURIFY
endif

INCLUDE		+= -I$(OSPL_HOME)/src/utilities/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/serialization/include
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/include
INCLUDE		+= -I$(OSPL_HOME)/src/user/include

LDLIBS		+= -l$(DDS_OS) -l$(DDS_UTIL) -l$(DDS_DATABASE) -l$(DDS_SERIALIZATION) -l$(DDS_KERNEL) -l$(DDS_USER)

-include $(DEPENDENCIES)
