TARGET_EXEC	:= cmsoap

include		$(OSPL_HOME)/setup/makefiles/target.mak

CINCS    += -I$(OSPL_HOME)/src/services/cmsoap/include

LDLIBS   += -l$(DDS_CMSOAP) -l$(DDS_CMXML) $(DDS_RRSTORAGE_LIBS) -l$(DDS_CORE) 
LDLIBS   += $(LDLIBS_CMS)

