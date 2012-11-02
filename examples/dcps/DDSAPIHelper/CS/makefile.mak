TARGET_CSLIB = DDSAPIHelper
CS_NAMESPCS	 = .
CODE_DIR	:= ../../src

# Specify the CSharp API on which this application depends.
CS_API = $(OSPL_HOME)/src/api/dcps/sacs/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)

# Fine tune the compiler flags.
CSLIBS += -r:$(CS_API)

include $(OSPL_HOME)/setup/makefiles/target.mak