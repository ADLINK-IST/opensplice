TARGET_DLIB     := cmsoap

CMS_HOME        =$(OSPL_HOME)/src/services/cmsoap

SOAP_MODULES_C  =soapC.c soapServer.c
SOAP_MODULES_H  =soapH.h soapStub.h
SOAP_MODULES    =$(SOAP_MODULES_H) $(SOAP_MODULES_C)

vpath %.h       ../../code

$(SOAP_MODULES): cms__soap.h
	$(SOAPCPP) -c $^

stdsoap2.c : $(ESCAPED_GSOAPHOME)/include/stdsoap2.c
	sed -e "s/^soap_LONG642s/__soap_LONG642s/" \
                -e "s/^soap_s2LONG64/__soap_s2LONG64/" \
                -e "s/^soap_ULONG642s/__soap_ULONG642s/" \
                -e "s/^soap_s2ULONG64/__soap_s2ULONG64/" "$<" > stdsoap2.c
	# cp $< .

C_FILES = $(notdir $(wildcard ../../code/*.c)) $(SOAP_MODULES_C) stdsoap2.c

include         $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS += -DOSPL_BUILD_CMSOAP
CPPFLAGS += -DWITH_FAST
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)

# Include dirs for this component
CINCS += -I"$(GSOAPHOME)/include"
CINCS += -I$(OSPL_HOME)/src/abstraction/include
CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/kernel/include
CINCS += -I$(OSPL_HOME)/src/user/include
CINCS += -I$(OSPL_HOME)/src/api/cm/xml/include
CINCS += -I$(OSPL_HOME)/src/api/cm/xml/code

ifneq (,$(findstring rtems,$(SPLICE_TARGET)))
CFLAGS += -D__GNU__
endif

ifneq (,$(findstring int5,$(SPLICE_TARGET)))
CFLAGS += -DCOMPILE_ENTRYPOINT_AS_MAIN
endif

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_CORE)
LDLIBS  += $(LDLIBS_CMS)    
LDLIBS  += -l$(DDS_CMXML) 
LDLIBS  += $(LDLIBS_CMS)

$(DEPENDENCIES): $(SOAP_MODULES)

-include $(DEPENDENCIES)
