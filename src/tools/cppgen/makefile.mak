#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= cppgen

include	$(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS += -l$(DDS_CPP) -l$(DDS_OS)

CXXINCS += -I$(OSPL_HOME)/src/cpp/include
CXXINCS += -I$(OSPL_HOME)/src/cpp/code
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/sacpp/include

-include $(DEPENDENCIES)
