# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_LOCCOLLECTIONS)

include	$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_LOC_COLLECTIONS
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS    += -I$(OSPL_HOME)/src/loc/util/include
CINCS    += -I$(OSPL_HOME)/src/abstraction/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_OS)
LDLIBS	+= -l$(DDS_LOCUTIL)

-include $(DEPENDENCIES)
