TARGET_EXEC     := nwbridge

include $(OSPL_HOME)/setup/makefiles/target.mak

CINCS   += -I$(OSPL_HOME)/src/services/nwbridge/include
CINCS   += -I$(OSPL_HOME)/src/services/nwbridge/bld/$(SPLICE_TARGET)

LDLIBS  += -l$(DDS_NWBRIDGE) -l$(DDS_CORE)
