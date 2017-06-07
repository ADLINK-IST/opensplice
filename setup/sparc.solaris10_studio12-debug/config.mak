# Set target context
PROC		 = sparc
OS		 = solaris
OS_REV		 = 10
SPECIAL		 = DEBUG

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/sparc.solaris10_studio12-default.mak
else
include $(OSPL_HOME)/setup/sparc.solaris10_studio12-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O0 -g
CFLAGS_DEBUG     = -DNDEBUG
CFLAGS_STRICT_UNINITIALIZED =
JCFLAGS          = -g

#Csc compiler flags
CSFLAGS_DEBUG    = -define:DEBUG\;TRACE -debug+ -debug:full
