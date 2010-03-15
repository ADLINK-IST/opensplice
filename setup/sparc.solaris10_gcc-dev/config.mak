# Set target context
PROC		 = sparc
OS		 = solaris
OS_REV		 = 10
SPECIAL		 = DEBUG

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/sparc.solaris10_gcc-default.mak
else
include $(OSPL_HOME)/setup/sparc.solaris10_gcc-default.mak
endif

# Compiler flags
# -DE_DEBUG is needed to enable debugging info for the SACPP apis
CFLAGS_DEBUG     = -g -D_TYPECHECK_ -DE_DEBUG
JCFLAGS	 	 = -g
