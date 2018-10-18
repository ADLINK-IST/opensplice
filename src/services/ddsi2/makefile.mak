TARGET_DLIB	:= ddsi2
ODL_MODULES	:= q_osplserModule

C_FILES_TO_FILTER := %_template.c
include	$(OSPL_HOME)/setup/makefiles/target.mak
-include $(OSPL_OUTER_HOME)/setup/makefiles/commercial.mak

##
## FIXME introduce LDLIBS_IN variable
LDLIBS += -l$(DDS_CORE) 
LDLIBS += $(LDLIBS_NW)

CPPFLAGS += -DOSPL_BUILD_DDSI2 -DMODEL_q_osplserModule_IMPLEMENTATION $(OSPL_OS_CPPFLAGS)

CFLAGS  += $(SHCFLAGS)

CFLAGS += $(CFLAGS_XSTRICT)
CFLAGS += $(CFLAGS_W_ERROR)
#ddsi_ssl$(OBJ_POSTFIX) ddsi_tcp$(OBJ_POSTFIX) q_sockwaitset$(OBJ_POSTFIX): CFLAGS += $(CFLAGS_W_NOERROR)

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)

# Include dirs for this component

CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/configuration/config/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
## workaround until v__networkQueue.h has moved from code/ to include/
CINCS += -I$(OSPL_HOME)/src/kernel/code
CINCS += -I$(OSPL_HOME)/src/user/code
CINCS += -I$(OSPL_HOME)/src/configuration/config/include


#CINCS += -I"$(OPENSSL_HOME)/include"
ifneq "$(WCECOMPAT)" ""
CINCS += "-I$(WCECOMPAT)/include"
endif

ifeq "$(INCLUDE_SECURITY)" "yes"
CPPFLAGS += -DDDSI_INCLUDE_SSL
CINCS += $(CINCS_NW_SEC)
LDLIBS += $(LDLIBS_NW_SEC)
endif

-include $(DEPENDENCIES)
