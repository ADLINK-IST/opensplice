# Set target context
PROC		 = x86
OS		 = linux
OS_REV		 =
SPECIAL		 = debug

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86.linux-default.mak
else
include $(OSPL_HOME)/setup/x86.linux-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O0 -g
CFLAGS_DEBUG     = -DNDEBUG
CFLAGS_STRICT_UNINITIALIZED =
JCFLAGS          = -g

#Csc compiler flags
CSFLAGS_DEBUG    = -define:DEBUG\;TRACE -debug+ -debug:full
