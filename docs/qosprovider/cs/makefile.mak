TARGET_CSEXEC  = QosProvider
CS_NAMESPCS	   = .

TARGET_LINK_DIR = ../../exec/$(SPLICE_TARGET)

# Specify the CSharp API and Data Model on which this application depends.
CS_API       = $(OSPL_HOME)/src/api/dcps/sacs/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)

# Fine tune the compiler flags.
CSLIBS += -r:$(CS_API)

include $(OSPL_HOME)/setup/makefiles/target.mak
