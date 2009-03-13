ifeq (,$(findstring shmt,$(SPLICE_TARGET)))
TARGET_EXEC	:= durability
ODL_MODULES	:= durabilityModule2

include		$(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS		+= -l$(DDS_USER) -l$(DDS_KERNEL) -l$(DDS_CONFPARSER) -l$(DDS_CONF) -l$(DDS_SERIALIZATION) -l$(DDS_DATABASE) -l$(DDS_UTIL) -l$(DDS_OS)

INCLUDE		+= -I$(OSPL_HOME)/src/database/serialization/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/include
INCLUDE		+= -I$(OSPL_HOME)/src/user/include
SPPODL_FLAGS    += -I$(OSPL_HOME)/src/kernel/code

-include $(DEPENDENCIES)

else

TARGET_DLIB     := durability
ODL_MODULES     := durabilityModule2

include         $(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS          += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS         += $(SHLDFLAGS)
LDLIBS          += $(SHLDLIBS)
LDLIBS          += -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) -l$(DDS_USER) -l$(DDS_SERIALIZATION)

INCLUDE         += -I$(OSPL_HOME)/src/database/serialization/include
INCLUDE         += -I$(OSPL_HOME)/src/database/database/include
INCLUDE         += -I$(OSPL_HOME)/src/kernel/include
INCLUDE         += -I$(OSPL_HOME)/src/user/include
SPPODL_FLAGS    += -I$(OSPL_HOME)/src/kernel/code

-include $(DEPENDENCIES)

endif
