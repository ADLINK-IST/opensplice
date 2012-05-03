# Set target context
PROC		 = x86_64
OS		     = linux
OS_REV		 = 2.6
SPECIAL		 = DEBUG

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/$(PROC).$(OS)$(OS_REV)-default.mak
else
include $(OSPL_HOME)/setup/$(PROC).$(OS)$(OS_REV)-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O0 -g
CFLAGS_DEBUG     = -DNDEBUG
JCFLAGS          = -g
