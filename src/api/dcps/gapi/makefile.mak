#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_DCPSGAPI)

include	$(OSPL_HOME)/setup/makefiles/target.mak

ifneq (,$(ALT_CFLAGS_OPT))
   CFLAGS_OPT=$(ALT_CFLAGS_OPT)
endif

CPPFLAGS	+= -DOSPL_BUILD_DCPSGAPI
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS		+= -I$(OSPL_HOME)/src/utilities/include
CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/database/serialization/include
CINCS		+= -I$(OSPL_HOME)/src/kernel/include
CINCS		+= -I$(OSPL_HOME)/src/user/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)

ifeq ($(SPECIAL),purify)
CPPFLAGS += -D_PURIFY
endif

LDLIBS += -l$(DDS_OS) -l$(DDS_UTIL) -l$(DDS_DATABASE) 
LDLIBS += -l$(DDS_SERIALIZATION) -l$(DDS_KERNEL) -l$(DDS_USER)

-include $(DEPENDENCIES)
