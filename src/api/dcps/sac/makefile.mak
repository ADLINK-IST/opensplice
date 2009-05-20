#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_DCPSSAC)

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_DCPSSAC
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS		+= -I../../../common/include
CINCS		+= -I../../../gapi/include
# the next include should not be needed! this is a design fault
CINCS		+= -I../../../gapi/code
CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/kernel/include
CINCS		+= -I$(OSPL_HOME)/src/user/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS	+= -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) 
LDLIBS	+= -l$(DDS_USER) -l$(DDS_DCPSGAPI)

-include $(DEPENDENCIES)
