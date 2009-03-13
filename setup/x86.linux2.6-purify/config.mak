# Set target context
PROC		 = x86
OS		 = linux
OS_REV		 = 2.6
SPECIAL		 = PURIFY

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86.linux-default.mak
else
include $(OSPL_HOME)/setup/x86.linux-default.mak
endif

# Compiler flags
CFLAGS_OPT       =
CFLAGS_DEBUG     = -g
JCFLAGS          = -g -Xlint:deprecation,unchecked,fallthrough,path,finally
CC		 = sh $(OSPL_HOME)/bin/splice_purify gcc
