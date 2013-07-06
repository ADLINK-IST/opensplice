#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_DLRLSAJ)

include	$(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS += $(SHCFLAGS) $(MTCFLAGS)
CINCS	 += -I$(OSPL_HOME)/src/loc/kernel/include
CINCS  += -I$(OSPL_HOME)/src/loc/kernel/include/bridge
CINCS  += -I$(OSPL_HOME)/src/loc/metamodel/include
CINCS  += -I$(OSPL_HOME)/src/loc/collections/include
CINCS  += -I$(OSPL_HOME)/src/loc/util/include
CINCS  += -I$(OSPL_HOME)/src/user/include
CINCS  += -I$(OSPL_HOME)/src/kernel/include
CINCS  += -I$(OSPL_HOME)/src/database/database/include
CINCS  += -I$(OSPL_HOME)/src/api/dcps/gapi/include
CINCS  += -I$(OSPL_HOME)/src/api/dcps/gapi/code
CINCS  += -I$(OSPL_HOME)/src/api/dcps/sac/include
CINCS  += -I$(OSPL_HOME)/src/api/dcps/java/common/c/include
CINCS  += $(JAVA_INCLUDE)

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_LOCKERNEL) -l$(DDS_LOCUTIL) -l$(DDS_LOCMETAMODEL)
LDLIBS  += -l$(DDS_LOCCOLLECTIONS) -l$(DDS_DCPSSAJ) -l$(DDS_DCPSSAC)
LDLIBS  += -l$(DDS_DCPSGAPI) -l$(DDS_USER) -l$(DDS_KERNEL)
LDLIBS  += -l$(DDS_DATABASE) -l$(DDS_OS)

-include $(DEPENDENCIES)
