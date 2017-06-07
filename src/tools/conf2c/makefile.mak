# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= osplconf2c

include		$(OSPL_HOME)/setup/makefiles/target.mak


ifneq (,$(or $(findstring _km,$(SPLICE_REAL_TARGET)), $(findstring vxworks653,$(SPLICE_REAL_TARGET))))
CFLAGS     += -DTARGET_VXWORKS_KM
endif

ifneq (,$(findstring vxworks5,$(SPLICE_REAL_TARGET)))
CFLAGS     += -DTARGET_VXWORKS_KM
endif

ifneq (,$(findstring int,$(SPLICE_REAL_TARGET)))
CFLAGS     += -DNO_DYNAMIC_LIB -DTARGET_INTEGRITY
endif

LDFLAGS     += $(LDFLAGS_FLEX)
LDLIBS		+= $(LDLIBS_FLEX)
LDLIBS += -l$(DDS_CORE)

CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/configuration/config/include
CINCS		+= -I$(OSPL_HOME)/src/configuration/parser/include

-include $(DEPENDENCIES)
