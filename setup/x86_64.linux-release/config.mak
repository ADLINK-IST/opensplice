# Set target context
PROC	     = x86_64
OS           = linux
OS_REV       =
SPECIAL      = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/$(PROC).$(OS)$(OS_REV)-default.mak
else
include $(OSPL_HOME)/setup/$(PROC).$(OS)$(OS_REV)-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O4 -fno-strict-aliasing $(CFLAGS_LTO)
CFLAGS_DEBUG     = -DNDEBUG
JCFLAGS          = -g:none -nowarn
