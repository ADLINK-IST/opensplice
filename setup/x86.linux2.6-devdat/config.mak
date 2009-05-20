# Set target context
PROC		 = x86
OS		 = linux
OS_REV		 = 2.6
SPECIAL		 = DEVDAT

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86.linux-default.mak
else
include $(OSPL_HOME)/setup/x86.linux-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O0
CFLAGS_DEBUG     = -g -D_TYPECHECK_ -DOBJECT_WALK
JCFLAGS          = -g
