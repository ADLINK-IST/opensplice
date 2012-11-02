#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= cppgen

include	$(OSPL_HOME)/setup/makefiles/target.mak

ifeq "$(OS)$(OS_REV)" "win32"
ifeq "$(SPECIAL)" "RELEASE"
#Reset to remove the -O2 which does not work
CFLAGS_OPT = -DNDEBUG -MD
endif
endif

LD_EXE = $(LD_CXX)  # c++ binary
LDLIBS += -l$(DDS_CPP) -l$(DDS_OS) $(LDLIBS_OS)
CXXFLAGS += -DYY_NEVER_INTERACTIVE

CXXINCS += -I$(OSPL_HOME)/src/cpp/include
CXXINCS += -I$(OSPL_HOME)/src/cpp/code
CXXINCS += -I$(OSPL_HOME)/src/api/dcps/sacpp/include

ifeq "$(CXX)" "g++"
LDFLAGS += -L.

.PHONY:

libstdc++.a:
	ln -sf `g++ -print-file-name=libstdc++.a`

cppgen:libstdc++.a
endif

-include $(DEPENDENCIES)
