# Set target context
PROC		 = x86
OS		 = darwin
OS_REV		 = 10
SPECIAL		 = DEBUG

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86.darwin10-default.mak
else
include $(OSPL_HOME)/setup/x86.darwin10-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O0 -DNDEBUG -fno-strict-aliasing
CFLAGS_DEBUG     = -g #-D_TYPECHECK_ -DE_DEBUG
JCFLAGS          = -g

# Csc compiler flags
CSFLAGS_DEBUG    = -define:DEBUG\;TRACE -debug+ -debug:full
