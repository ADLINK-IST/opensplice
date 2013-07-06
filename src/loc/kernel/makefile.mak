# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_LOCKERNEL)
TARGET_LINK_DIR := $(SPLICE_LIBRARY_PATH)

include	$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_LOC_KERNEL
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/include
CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/kernel/include
CINCS		+= -I$(OSPL_HOME)/src/kernel/code
CINCS		+= -I$(OSPL_HOME)/src/user/include
CINCS		+= -I$(OSPL_HOME)/src/user/code
CINCS		+= -I$(OSPL_HOME)/src/loc/util/include
CINCS		+= -I$(OSPL_HOME)/src/loc/collections/include
CINCS		+= -I$(OSPL_HOME)/src/loc/metamodel/include
CINCS		+= -I$(OSPL_HOME)/src/loc/kernel/code/include
CINCS		+= -I$(OSPL_HOME)/src/loc/kernel/include/bridge
CINCS       += -I$(OSPL_HOME)/src/configuration/config/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) -l$(DDS_USER)
LDLIBS	+= -l$(DDS_LOCUTIL) -l$(DDS_LOCCOLLECTIONS) -l$(DDS_LOCMETAMODEL)

#CINCS	 += -I$(DCPS_SOURCE)/components/api/dcps/gapi/code
#CINCS	 += -I$(DCPS_SOURCE)/components/user/code
#CINCS	 += -I$(DCPS_SOURCE)/components/kernel/code

-include $(DEPENDENCIES)
