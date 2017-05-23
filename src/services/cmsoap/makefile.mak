.NOTPARALLEL:

TARGET_DLIB     := cmsoap

CMS_HOME        =$(OSPL_HOME)/src/services/cmsoap

SOAP_MODULES_C  =soapC.c soapServer.c
SOAP_MODULES_H  =soapH.h soapStub.h
SOAP_MODULES    =$(SOAP_MODULES_H) $(SOAP_MODULES_C)

vpath %.h       ../../code

$(SOAP_MODULES): cms__soap.h
	$(SOAPCPP) -c $^ >soapcpp.output 2>&1 || { xc=$? ; cat soapcpp.output ; exit $xc ; }
	mv soapC.c soapC.c.orig
	(echo '#ifndef __clang_analyzer__' ; cat soapC.c.orig ; echo '#endif') > soapC.c

stdsoap2.c : $(ESCAPED_GSOAPHOME)/include/stdsoap2.c
	(echo '#ifndef __clang_analyzer__' ; \
	 sed -e "s/^soap_LONG642s/__soap_LONG642s/" \
                -e "s/^soap_s2LONG64/__soap_s2LONG64/" \
                -e "s/^soap_ULONG642s/__soap_ULONG642s/" \
                -e "s/^soap_s2ULONG64/__soap_s2ULONG64/" "$<" ; \
	 echo '#endif') > stdsoap2.c

ifneq (,$(findstring integrity,$(OS)))
		@echo "Patching gsoap for integrity"
		cp $(ESCAPED_GSOAPHOME)/include/stdsoap2.h .
		patch stdsoap2.c $(OSPL_OUTER_HOME)/src/services/cmsoap_licensed/soap_integrity_c.patch
		patch stdsoap2.h $(OSPL_OUTER_HOME)/src/services/cmsoap_licensed/soap_integrity_h.patch
endif

C_FILES = $(notdir $(wildcard ../../code/*.c)) $(SOAP_MODULES_C) stdsoap2.c

include         $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS += -DOSPL_BUILD_CMSOAP
CPPFLAGS += -DWITH_FAST
CFLAGS   += $(SHCFLAGS)
CFLAGS   += $(CFLAGS_XSTRICT)

stdsoap2$(OBJ_POSTFIX) soapC$(OBJ_POSTFIX): CFLAGS := $(filter-out $(CFLAGS_XSTRICT), $(CFLAGS)) $(CFLAGS_PERMISSIVE)

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

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)
LDLIBS  += -l$(DDS_CORE)
LDLIBS  += $(LDLIBS_CMS)    
LDLIBS  += -l$(DDS_CMXML) 
LDLIBS  += $(LDLIBS_CMS)

$(DEPENDENCIES): $(SOAP_MODULES)

-include $(DEPENDENCIES)
