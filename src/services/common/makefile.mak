# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_COMMONSERV)

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_REPORTSERVICE
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)

CINCS    += -I$(OSPL_HOME)/src/database/database/include

LDLIBS	+= -l$(DDS_OS)

-include $(DEPENDENCIES)
