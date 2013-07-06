# Set target context - there is no meaningful difference
# between Solaris 11 and Solaris 10, so just use the 
# Solaris 10 abstraction layer
PROC		 = x86_64
OS		 = solaris
OS_REV		 = 10
SPECIAL		 = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86_64.solaris11_studio12-default.mak
else
include $(OSPL_HOME)/setup/x86_64.solaris11_studio12-default.mak
endif

# Compiler flags
# -DE_DEBUG is needed to enable debugging info for the SACPP apis
CFLAGS_DEBUG     = -g #-D_TYPECHECK_ -DE_DEBUG
CFLAGS_OPT       = -g -fast -v -DNDEBUG

JCFLAGS	 	 = -g
