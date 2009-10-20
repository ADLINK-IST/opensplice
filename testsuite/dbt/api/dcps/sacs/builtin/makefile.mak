#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_CSEXEC  = SACSTesterBuiltin
CS_NAMESPCS	   = test test/sacs
DATA_MODEL     = test_type
TEST_FRAMEWORK = testframe

TARGET_LINK_DIR = ../../../exec/$(SPLICE_TARGET)

all link: csapi csc rmapi

include $(OSPL_HOME)/setup/makefiles/target.mak

# Specify the CSharp API and Data Model on which this application depends.
CS_API       := $(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)
CS_MODEL     := $(CSLIB_PREFIX)$(DATA_MODEL)$(CSLIB_POSTFIX)
CS_FRAMEWORK := $(CSLIB_PREFIX)$(TEST_FRAMEWORK)$(CSLIB_POSTFIX)

# Add the generated files to the compiler-list.
CS_FILES += $(IDLPP_CS)

# Fine tune the compiler flags.
CSLIBS += -r:$(CS_API) -r:$(CS_MODEL) -r:$(CS_FRAMEWORK)

csapi: $(TARGET_LINK_DIR) $(CS_API) $(CS_MODEL) $(CS_FRAMEWORK)

# Copy the C# API next to the executable (required when not yet copied to GAC)
$(CS_API):
	cp $(OSPL_HOME)/src/api/dcps/sacs/bld/$(SPLICE_TARGET)/$(CS_API) .
	
# Copy the C# Data model next to the executable (required when not yet copied to GAC)
$(CS_MODEL):
	cp $(TARGET_LINK_DIR)/$(CS_MODEL) .
	
# Copy the C# test framework next to the executable (required when not yet copied to GAC)
$(CS_FRAMEWORK):
	cp $(OSPL_HOME)/testsuite/framework/cstestframe/bld/$(SPLICE_TARGET)/$(CS_FRAMEWORK) .
	cp $(OSPL_HOME)/testsuite/framework/cstestframe/bld/$(SPLICE_TARGET)/$(CS_FRAMEWORK) $(TARGET_LINK_DIR)

# Remove the C# API again to remove unnecessary redundancy.
rmapi:
	rm $(CS_API)
	rm $(CS_MODEL)
	rm $(CS_FRAMEWORK)

