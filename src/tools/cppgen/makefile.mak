#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= cppgen

include	$(OSPL_HOME)/setup/makefiles/target.mak


ifdef WIN32
ifdef OSPL_ENV_DEBUG
#Reset to remove the -O2 which does not work 
CFLAGS_OPT = -DNDEBUG -MD
endif
endif

LDLIBS += -l$(DDS_CPP) -l$(DDS_OS)
CXXFLAGS += -DYY_NEVER_INTERACTIVE

CXXINCS += -I$(OSPL_HOME)/src/cpp/include
CXXINCS += -I$(OSPL_HOME)/src/cpp/code
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/sacpp/include

-include $(DEPENDENCIES)
