TARGET_CSEXEC  = SACSDefaultConstructor
CS_NAMESPCS    = test test/sacs
TEST_FRAMEWORK = cstestframe

TARGET_LINK_DIR = ../../../exec/$(SPLICE_TARGET)

# Specify the location of the IDL data model, and the generation results of idlpp.
IDL_DIR     := ../../idl/code
TOPIC_IDL   := $(notdir $(wildcard $(IDL_DIR)/*.idl))
IDL_CS    := $(TOPIC_IDL:%.idl=%.cs) $(TOPIC_IDL:%.idl=I%Dcps.cs) $(TOPIC_IDL:%.idl=%Dcps.cs) $(TOPIC_IDL:%.idl=%SplDcps.cs)

# Specify the CSharp API and Data Model on which this application depends.
CS_API       = $(OSPL_HOME)/src/api/dcps/sacs/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)
CS_FRAMEWORK = $(OSPL_HOME)/testsuite/framework/cstestframe/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(TEST_FRAMEWORK)$(CSLIB_POSTFIX)

# Fine tune the compiler flags.
CSLIBS += -r:$(CS_API),$(CS_FRAMEWORK)

include $(OSPL_HOME)/setup/makefiles/target.mak

all link: $(CS_FRAMEWORK)

# Make sure the framework has been compiled first.
$(CS_FRAMEWORK):
	cd $(OSPL_HOME)/testsuite/framework/cstestframe; make

# Generate the C++ interfaces from the IDL descriptions.
$(IDL_CS): $(IDL_DIR)/$(TOPIC_IDL)
	idlpp -l cs -S $<
