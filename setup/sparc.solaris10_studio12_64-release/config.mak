# Set target context
PROC		 = sparc
OS		 = solaris
OS_REV		 = 10
SPECIAL		 = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/sparc.solaris10_studio12_64-default.mak
else
include $(OSPL_HOME)/setup/sparc.solaris10_studio12_64-default.mak
endif


# Compiler flags
CFLAGS_DEBUG     =
JCFLAGS		 = -g:none -nowarn

