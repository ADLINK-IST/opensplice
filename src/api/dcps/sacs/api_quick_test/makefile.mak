#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_CSEXEC = TestBinding
CS_NAMESPCS	 = . Data

all link: csc

include $(OSPL_HOME)/setup/makefiles/target.mak

CSLIBS += -L$(SPLICE_LIBRARY_PATH) -ldcpssacs