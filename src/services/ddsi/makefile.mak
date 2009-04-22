#
# included by bld/$(SPLICE_HOST)/makefile

ifeq (,$(findstring shmt,$(SPLICE_TARGET)))
TARGET_EXEC	:= ddsi

include		$(OSPL_HOME)/setup/makefiles/target.mak

##
## FIXME introduce LDLIBS_IN variable
LDLIBS		+= -l$(DDS_OS)
LDLIBS		+= -l$(DDS_DATABASE)
LDLIBS		+= -l$(DDS_KERNEL)
LDLIBS		+= -l$(DDS_USER)
LDLIBS		+= -l$(DDS_SERIALIZATION)
LDLIBS		+= $(LDLIBS_NW)

# Suppression of macro's (QAC)

CPFLAGS         += -macros ../../nw_macros.qac
# Include dirs for this component

INCLUDE		+= -I$(OSPL_HOME)/src/user/include
INCLUDE		+= -I$(OSPL_HOME)/src/user/code
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/serialization/include
INCLUDE		+= -I$(OSPL_HOME)/src/utilities/include
## workarround until v__networkQueue.h has moved from code/ to include/
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/code

-include $(DEPENDENCIES)

else

TARGET_DLIB	:= ddsi

include         $(OSPL_HOME)/setup/makefiles/target.mak
CFLAGS          += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS         += $(SHLDFLAGS)
LDLIBS          += $(SHLDLIBS)
LDLIBS          += -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) -l$(DDS_USER) -l$(DDS_SERIALIZATION) $(LDLIBS_NW)

# Suppression of macro's (QAC)

CPFLAGS         += -macros ../../nw_macros.qac

# Include dirs for this component

INCLUDE         += -I$(OSPL_HOME)/src/user/include
#INCLUDE		+= -I$(OSPL_OUTER_HOME)/src/loc/collections/include
INCLUDE         += -I$(OSPL_HOME)/src/kernel/include
INCLUDE         += -I$(OSPL_HOME)/src/database/database/include
INCLUDE         += -I$(OSPL_HOME)/src/database/serialization/include
INCLUDE		+= -I$(OSPL_HOME)/src/utilities/include


-include $(DEPENDENCIES)

endif
