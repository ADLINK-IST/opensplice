TARGET_CSEXEC = BuiltInTopicsDataSubscriber
CS_NAMESPCS	 = .
CODE_DIR	:= ../../src

# Specify the CSharp API on which this application depends.
CS_API = $(OSPL_HOME)/src/api/dcps/sacs/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)
CS_API_HELPER = $(OSPL_HOME)/examples/dcps/DDSAPIHelper/CS/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)DDSAPIHelper$(CSLIB_POSTFIX)

# Fine tune the compiler flags.
CSLIBS += -r:$(CS_API) -r:$(CS_API_HELPER)

include $(OSPL_HOME)/setup/makefiles/target.mak