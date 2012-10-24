#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= idlpp

include	$(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += -l$(DDS_CPP) -l$(DDS_UTIL) -l$(DDS_SERIALIZATION)
LDLIBS += -l$(DDS_DATABASE) -l$(DDS_OS)

CINCS += -I$(OSPL_HOME)/src/database/database/include
CINCS += -I$(OSPL_HOME)/src/database/serialization/include
CINCS += -I$(OSPL_HOME)/src/utilities/include
CINCS += -I$(OSPL_HOME)/src/cpp/include
CINCS += -I$(OSPL_HOME)/src/cppgen/include

include	$(OSPL_HOME)/src/tools/idlpp/makefile_templates.mak

ifneq (,$(ALT_CFLAGS_OPT))
   CFLAGS_OPT=$(ALT_CFLAGS_OPT)
endif

$(ACE_TAO_1_4_1_TMPLS): $(CCPP_TEMPLATES)
	cp $(CCPP_TEMPLATES) $(CCPP_TMPL_PATH)/DDS_ACE_TAO_1_4_1

$(MICO_2_3_11_TMPLS): $(CCPP_TEMPLATES)
	cp $(CCPP_TEMPLATES) $(CCPP_TMPL_PATH)/DDS_Mico_2_3_11

$(EORB_3_0_TMPLS): $(CCPP_TEMPLATES)
	cp $(CCPP_TEMPLATES) $(CCPP_TMPL_PATH)/DDS_Eorb_3_0

$(OPENFUSION_1_4_1_TMPLS): $(CCPP_TEMPLATES)
	cp $(CCPP_TEMPLATES) $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_4_1

$(OPENFUSION_1_5_1_TMPLS): $(CCPP_TEMPLATES)
	cp $(CCPP_TEMPLATES) $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_5_1

$(OPENFUSION_1_6_1_TMPLS): $(CCPP_TEMPLATES)
	cp $(CCPP_TEMPLATES) $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_6_1

$(SACPP_TMPLS): $(CCPP_TEMPLATES)
	cp $(CCPP_TEMPLATES) $(SACPP_TMPL_PATH)

$(DEPENDENCIES): $(ORB_TEMPLATES)

-include $(DEPENDENCIES)
