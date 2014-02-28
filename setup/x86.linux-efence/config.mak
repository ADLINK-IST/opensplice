# Set target context
PROC		 = x86
OS		 = linux
OS_REV		 =
SPECIAL		 = EFENCE

LDLIBS		 = -lefence -lc -lm -lpthread
include $(OSPL_OUTER_HOME)/setup/x86.linux-default.mak
else
include $(OSPL_HOME)/setup/x86.linux-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O0
CFLAGS_DEBUG     = -g
CFLAGS_STRICT_UNINITIALIZED =
JCFLAGS          = -g -Xlint:deprecation,unchecked,fallthrough,path,finally
