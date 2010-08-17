# Set target context
PROC		 = arm9
OS           = linux
OS_REV		 = 2.6
SPECIAL		 = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/arm9.linux-default.mak
else
include $(OSPL_HOME)/setup/arm9.linux-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O4
CFLAGS_DEBUG     = -DNDEBUG
JCFLAGS          = -g:none -nowarn
