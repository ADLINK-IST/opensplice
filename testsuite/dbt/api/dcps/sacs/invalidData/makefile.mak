TARGET_CSEXEC  = SACSTesterInvalidData
CS_NAMESPCS	   = test test/sacs
DATA_MODEL     = test_type
TEST_FRAMEWORK = cstestframe

TARGET_LINK_DIR = ../../../exec/$(SPLICE_TARGET)

# Specify the CSharp API and Data Model on which this application depends.
CS_API       = $(OSPL_HOME)/src/api/dcps/sacs/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)
CS_MODEL     = $(OSPL_HOME)/testsuite/dbt/api/dcps/sacs/common/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(DATA_MODEL)$(CSLIB_POSTFIX)
CS_FRAMEWORK = $(OSPL_HOME)/testsuite/framework/cstestframe/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(TEST_FRAMEWORK)$(CSLIB_POSTFIX)

# Fine tune the compiler flags.
CSLIBS += -r:$(CS_API),$(CS_MODEL),$(CS_FRAMEWORK)

include $(OSPL_HOME)/setup/makefiles/target.mak
