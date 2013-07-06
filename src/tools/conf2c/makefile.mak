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
LDLIBS		+= -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_CONF) -l$(DDS_CONFPARSER) $(LDLIBS_FLEX)

CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/configuration/config/include
CINCS		+= -I$(OSPL_HOME)/src/configuration/parser/include

-include $(DEPENDENCIES)
