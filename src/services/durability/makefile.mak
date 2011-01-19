ifeq (,$(findstring shmt,$(SPLICE_TARGET)))
TARGET_EXEC	:= durability
ODL_MODULES	:= durabilityModule2

ifneq (,$(findstring win,$(SPLICE_TARGET)))
ODL_MODULES	+= kernelModule
endif

include $(OSPL_HOME)/setup/makefiles/target.mak

CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/user/include

SPPODL_FLAGS += -I$(OSPL_HOME)/src/kernel/code

LDLIBS += -l$(DDS_USER) -l$(DDS_KERNEL) -l$(DDS_CONFPARSER) 
LDLIBS += -l$(DDS_CONF) -l$(DDS_SERIALIZATION) -l$(DDS_DATABASE) 
LDLIBS += -l$(DDS_UTIL) -l$(DDS_OS)


-include $(DEPENDENCIES)

else 

TARGET_DLIB := durability
ODL_MODULES := durabilityModule2

ifneq (,$(findstring win,$(SPLICE_TARGET)))
ODL_MODULES	+= kernelModule
endif

include $(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS += $(SHCFLAGS) $(MTCFLAGS)
CINCS  += -I$(OSPL_HOME)/src/database/serialization/include
CINCS  += -I$(OSPL_HOME)/src/database/database/include
CINCS  += -I$(OSPL_HOME)/src/kernel/include
CINCS  += -I$(OSPL_HOME)/src/user/include

SPPODL_FLAGS += -I$(OSPL_HOME)/src/kernel/code

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) 
LDLIBS  += -l$(DDS_USER) -l$(DDS_SERIALIZATION)

-include $(DEPENDENCIES)

endif
