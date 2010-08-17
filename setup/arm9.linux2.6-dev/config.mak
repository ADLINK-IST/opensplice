# Set target context
PROC		 = arm9
OS           = linux
OS_REV		 = 2.6
SPECIAL		 = DEBUG

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/arm9.linux-default.mak
else
include $(OSPL_HOME)/setup/arm9.linux-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O0
# -DE_DEBUG is needed to enable debugging info for the SACPP apis
CFLAGS_DEBUG     = -g -D_TYPECHECK_ -DE_DEBUG
JCFLAGS          = -g
