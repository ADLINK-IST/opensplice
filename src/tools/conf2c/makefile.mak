# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= osplconf2c

include		$(OSPL_HOME)/setup/makefiles/target.mak


ifneq (,$(or $(findstring _km,$(SPLICE_REAL_TARGET)), $(findstring vxworks653,$(SPLICE_REAL_TARGET))))
CFLAGS     += -DTARGET_VXWORKS_KM
endif

ifneq (,$(findstring vxworks5,$(SPLICE_REAL_TARGET)))
CFLAGS     += -DTARGET_VXWORKS_KM
endif

LDFLAGS     += $(LDFLAGS_FLEX)
LDLIBS		+= $(LDLIBS_FLEX)
#if we are building for the host then use ddshts lib
#as there is no ddskernel
ifneq ($(SPLICE_TARGET),$(SPLICE_REAL_TARGET))
LDLIBS += -l$(DDS_HTS)
else
LDLIBS += -l$(DDS_CORE)
endif

CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/configuration/config/include
CINCS		+= -I$(OSPL_HOME)/src/configuration/parser/include

-include $(DEPENDENCIES)
