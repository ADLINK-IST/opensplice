#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB		:= $(DDS_DCPSSAC)

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_DCPSSAC
CFLAGS          += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS         += $(SHLDFLAGS)
LDLIBS          += $(SHLDLIBS)

INCLUDE		+= -I../../../common/include
INCLUDE		+= -I../../../gapi/include
# the next include should not be needed! this is a design fault
INCLUDE		+= -I../../../gapi/code
INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/include
INCLUDE		+= -I$(OSPL_HOME)/src/user/include

LDLIBS		+= -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) -l$(DDS_USER) -l$(DDS_DCPSGAPI)

-include $(DEPENDENCIES)
