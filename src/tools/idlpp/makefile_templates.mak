#
# included by makefile and makefile.mak

CCPP_TMPL_PATH		:=$(OSPL_HOME)/etc/idlpp/CCPP
SACPP_TMPL_PATH     :=$(OSPL_HOME)/etc/idlpp/SACPP
ORB_INDEP_DIR		:=orb_independent

CCPP_TMPLS		:=  corbaCxxClassBody
CCPP_TMPLS		+= corbaCxxClassBodyHeader
CCPP_TMPLS		+= corbaCxxClassSpec
CCPP_TMPLS		+= corbaCxxClassSpecHeader
CCPP_TMPLS		+= corbaCxxMetaDescription
CCPP_TMPLS      += ISOCxxClassSpecHeader

CCPP_TEMPLATES 		=$(foreach t,$(CCPP_TMPLS), $(addprefix $(CCPP_TMPL_PATH)/$(ORB_INDEP_DIR)/, $(t)))

ACE_TAO_1_4_1_TMPLS	= $(foreach t, $(CCPP_TMPLS), $(addprefix $(CCPP_TMPL_PATH)/DDS_ACE_TAO_1_4_1/, $(t)))
MICO_2_3_11_TMPLS 	= $(foreach t, $(CCPP_TMPLS), $(addprefix $(CCPP_TMPL_PATH)/DDS_Mico_2_3_11/, $(t)))
EORB_3_0_TMPLS 		= $(foreach t, $(CCPP_TMPLS), $(addprefix $(CCPP_TMPL_PATH)/DDS_Eorb_3_0/, $(t)))
OPENFUSION_1_4_1_TMPLS	= $(foreach t, $(CCPP_TMPLS), $(addprefix $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_4_1/, $(t)))
OPENFUSION_1_5_1_TMPLS	= $(foreach t, $(CCPP_TMPLS), $(addprefix $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_5_1/, $(t)))
OPENFUSION_1_6_1_TMPLS	= $(foreach t, $(CCPP_TMPLS), $(addprefix $(CCPP_TMPL_PATH)/DDS_OpenFusion_1_6_1/, $(t)))
OPENFUSION_2_TMPLS	= $(foreach t, $(CCPP_TMPLS), $(addprefix $(CCPP_TMPL_PATH)/DDS_OpenFusion_2/, $(t)))
DDS_ACE_TAO_5_6_6_TMPLS	= $(foreach t, $(CCPP_TMPLS), $(addprefix $(CCPP_TMPL_PATH)/DDS_ACE_TAO_5_6_6/, $(t)))
SACPP_TMPLS             = $(foreach t, $(CCPP_TMPLS), $(addprefix $(SACPP_TMPL_PATH)/, $(t)))

ORB_TEMPLATES = $(ACE_TAO_1_4_1_TMPLS) $(MICO_2_3_11_TMPLS) $(EORB_3_0_TMPLS) $(OPENFUSION_1_4_1_TMPLS) $(OPENFUSION_1_5_1_TMPLS) $(OPENFUSION_1_6_1_TMPLS) $(OPENFUSION_2_TMPLS) $(SACPP_TMPLS) $(DDS_ACE_TAO_5_6_6_TMPLS)

