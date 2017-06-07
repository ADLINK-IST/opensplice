# Set target context
PROC		 = sparc
OS		 = solaris
OS_REV		 = 10
SPECIAL		 = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/sparc.solaris10_studio12-default.mak
else
include $(OSPL_HOME)/setup/sparc.solaris10_studio12-default.mak
endif


# Compiler flags
CFLAGS_DEBUG     =
JCFLAGS		 = -g -nowarn

