# Copy Streams idlpp templates to $(OSPL_HOME)/etc/idlpp

# Sources
ORB_DEP_TMPL 		:= corbaCxxStreamsMainInclude
ORB_INDEP_TMPLS		:= corbaCxxStreamsClassSpecHeader \
					   corbaCxxStreamsClassSpec \
				   	   corbaCxxStreamsClassBodyHeader \
				   	   corbaCxxStreamsClassBody \
				   	   corbaStreamsIdlInterfaceHeader \
				   	   corbaStreamsIdlInterface
ISOCXX_TMPLS		:= ISOCxxStreamsIdlInterface \
					   ISOCxxStreamsIdlInterfaceHeader
ORB_INDEP_DIR		:= CCPP/orb_independent

# Destinations
CCPP_TMPL_PATH		:= $(OSPL_HOME)/etc/idlpp/CCPP
SACPP_TMPL_PATH 	:= $(OSPL_HOME)/etc/idlpp/SACPP

ACE_TAO_1_4_1_TMPLS		= $(foreach t, $(ORB_INDEP_TMPLS) $(ORB_DEP_TMPL), $(addprefix $(CCPP_TMPL_PATH)/DDS_ACE_TAO_1_4_1/, 	$(t)))
MICO_2_3_11_TMPLS 		= $(foreach t, $(ORB_INDEP_TMPLS) $(ORB_DEP_TMPL), $(addprefix $(CCPP_TMPL_PATH)/DDS_Mico_2_3_11/,		$(t)))
EORB_3_0_TMPLS 			= $(foreach t, $(ORB_INDEP_TMPLS) $(ORB_DEP_TMPL), $(addprefix $(CCPP_TMPL_PATH)/DDS_Eorb_3_0/,			$(t)))
OPENFUSION_1_4_1_TMPLS	= $(foreach t, $(ORB_INDEP_TMPLS) $(ORB_DEP_TMPL), $(addprefix $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_4_1/,	$(t)))
OPENFUSION_1_5_1_TMPLS	= $(foreach t, $(ORB_INDEP_TMPLS) $(ORB_DEP_TMPL), $(addprefix $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_5_1/,	$(t)))
OPENFUSION_1_6_1_TMPLS	= $(foreach t, $(ORB_INDEP_TMPLS) $(ORB_DEP_TMPL), $(addprefix $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_6_1/,	$(t)))
OPENFUSION_2_TMPLS	= $(foreach t, $(ORB_INDEP_TMPLS) $(ORB_DEP_TMPL), $(addprefix $(CCPP_TMPL_PATH)/DDS_OpenFusion_2/,	$(t)))
DDS_ACE_TAO_5_6_6_TMPLS	= $(foreach t, $(ORB_INDEP_TMPLS) $(ORB_DEP_TMPL), $(addprefix $(CCPP_TMPL_PATH)/DDS_ACE_TAO_5_6_6/,	$(t)))
SACPP_TMPLS             = $(foreach t, $(ORB_INDEP_TMPLS) $(ORB_DEP_TMPL) $(ISOCXX_TMPLS), $(addprefix $(SACPP_TMPL_PATH)/, 					$(t)))

ORB_TEMPLATES           = $(ACE_TAO_1_4_1_TMPLS) $(MICO_2_3_11_TMPLS) $(EORB_3_0_TMPLS) $(OPENFUSION_1_4_1_TMPLS) $(OPENFUSION_1_5_1_TMPLS) $(OPENFUSION_1_6_1_TMPLS) $(OPENFUSION_2_TMPLS) $(DDS_ACE_TAO_5_6_6_TMPLS) $(SACPP_TMPLS)

all link: $(ORB_TEMPLATES)

$(CCPP_TMPL_PATH)/DDS_ACE_TAO_1_4_1/%: $(ORB_INDEP_DIR)/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_ACE_TAO_1_4_1" 
	@cp $? $(CCPP_TMPL_PATH)/DDS_ACE_TAO_1_4_1

$(CCPP_TMPL_PATH)/DDS_ACE_TAO_1_4_1/%: CCPP/DDS_ACE_TAO_1_4_1/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_ACE_TAO_1_4_1"
	@cp $? $(CCPP_TMPL_PATH)/DDS_ACE_TAO_1_4_1

$(CCPP_TMPL_PATH)/DDS_Mico_2_3_11/%: $(ORB_INDEP_DIR)/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_Mico_2_3_11"
	@cp $? $(CCPP_TMPL_PATH)/DDS_Mico_2_3_11

$(CCPP_TMPL_PATH)/DDS_Mico_2_3_11/%: CCPP/DDS_Mico_2_3_11/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_Mico_2_3_11"
	@cp $? $(CCPP_TMPL_PATH)/DDS_Mico_2_3_11

$(CCPP_TMPL_PATH)/DDS_Eorb_3_0/%: $(ORB_INDEP_DIR)/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_Eorb_3_0"
	@cp $? $(CCPP_TMPL_PATH)/DDS_Eorb_3_0

$(CCPP_TMPL_PATH)/DDS_Eorb_3_0/%: CCPP/DDS_Eorb_3_0/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_Eorb_3_0"
	@cp $? $(CCPP_TMPL_PATH)/DDS_Eorb_3_0

$(CCPP_TMPL_PATH)/DDS_OpenFusion_1_4_1/%: $(ORB_INDEP_DIR)/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_4_1"
	@cp $? $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_4_1

$(CCPP_TMPL_PATH)/DDS_OpenFusion_1_4_1/%: CCPP/DDS_OpenFusion_1_4_1/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_4_1"
	@cp $? $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_4_1

$(CCPP_TMPL_PATH)/DDS_OpenFusion_1_5_1/%: $(ORB_INDEP_DIR)/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_5_1"
	@cp $? $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_5_1

$(CCPP_TMPL_PATH)/DDS_OpenFusion_1_5_1/%: CCPP/DDS_OpenFusion_1_5_1/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_5_1"
	@cp $? $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_5_1

$(CCPP_TMPL_PATH)/DDS_OpenFusion_1_6_1/%: $(ORB_INDEP_DIR)/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_6_1"
	@cp $? $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_6_1

$(CCPP_TMPL_PATH)/DDS_OpenFusion_1_6_1/%: CCPP/DDS_OpenFusion_1_6_1/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_6_1"
	@cp $? $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_6_1

$(CCPP_TMPL_PATH)/DDS_OpenFusion_2/%: $(ORB_INDEP_DIR)/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_OpenFusion_2"
	@cp $? $(CCPP_TMPL_PATH)/DDS_OpenFusion_2

$(CCPP_TMPL_PATH)/DDS_OpenFusion_2/%: CCPP/DDS_OpenFusion_2/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_OpenFusion_2"
	@cp $? $(CCPP_TMPL_PATH)/DDS_OpenFusion_2

$(CCPP_TMPL_PATH)/DDS_ACE_TAO_5_6_6/%: $(ORB_INDEP_DIR)/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_ACE_TAO_5_6_6"
	@cp $? $(CCPP_TMPL_PATH)/DDS_ACE_TAO_5_6_6

$(CCPP_TMPL_PATH)/DDS_ACE_TAO_5_6_6/%: CCPP/DDS_ACE_TAO_5_6_6/%
	@echo "Copying $? -> $(CCPP_TMPL_PATH)/DDS_ACE_TAO_5_6_6"
	@cp $? $(CCPP_TMPL_PATH)/DDS_ACE_TAO_5_6_6

$(SACPP_TMPL_PATH)/%: $(ORB_INDEP_DIR)/%
	@echo "Copying $? -> $(SACPP_TMPL_PATH)"
	@cp $? $(SACPP_TMPL_PATH)

$(SACPP_TMPL_PATH)/%: SACPP/%
	@echo "Copying $? -> $(SACPP_TMPL_PATH)"
	@cp $? $(SACPP_TMPL_PATH)

clean:
	rm -f $(ORB_TEMPLATES)
