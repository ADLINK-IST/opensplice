# Set target context
PROC		 = x86_64
OS		 = darwin
OS_REV		 = 10
SPECIAL		 = 

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86_64.darwin10_clang-default.mak
else
include $(OSPL_HOME)/setup/x86_64.darwin10_clang-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O4 -fno-strict-aliasing
CFLAGS_DEBUG     = -g -DNDEBUG #-D_TYPECHECK_ -DE_DEBUG
JCFLAGS          = -g:none -nowarn

# Csc compiler flags
CSFLAGS_DEBUG    = -define:DEBUG\;TRACE -debug+ -debug:full
