# Set target context
PROC		 = sparc
OS		 = solaris
OS_REV		 = 9
SPECIAL		 = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/sparc.solaris9_gcc-default.mak
else
include $(OSPL_HOME)/setup/sparc.solaris9_gcc-default.mak
endif


# Compiler flags
CFLAGS_DEBUG     =
JCFLAGS		 = -g:none -nowarn

