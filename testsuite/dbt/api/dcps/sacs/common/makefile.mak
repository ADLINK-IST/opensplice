TARGET_CSLIB = test_type
CS_NAMESPCS	 = test/sacs

TARGET_LINK_DIR = ../../../exec/$(SPLICE_TARGET)

# Specify the location of the IDL data model, and the generation results of idlpp.
IDL_DIR     := ../../code
TOPIC_IDL   := $(notdir $(wildcard $(IDL_DIR)/*.idl))
IDL_CS    := $(TOPIC_IDL:%.idl=%.cs) $(TOPIC_IDL:%.idl=I%Dcps.cs) $(TOPIC_IDL:%.idl=%Dcps.cs) $(TOPIC_IDL:%.idl=%SplDcps.cs)

# Specify the CSharp API on which this application depends.
CS_API = $(OSPL_HOME)/src/api/dcps/sacs/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)

# Fine tune the compiler flags.
CSLIBS += -r:$(CS_API)

include $(OSPL_HOME)/setup/makefiles/target.mak

# Generate the C++ interfaces from the IDL descriptions.
$(IDL_CS): $(IDL_DIR)/$(TOPIC_IDL)
	idlpp -l cs -S $<
