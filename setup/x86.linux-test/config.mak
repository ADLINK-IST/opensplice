# Set target context
PROC		 = x86
OS		 = linux
OS_REV		 =
SPECIAL		 = TEST

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86.linux-default.mak
else
include $(OSPL_HOME)/setup/x86.linux-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O0
CFLAGS_DEBUG     = -gstabs3 -DNDEBUG -fprofile-arcs -ftest-coverage 
CFLAGS_STRICT	 = -Wall -W -pedantic -Wwrite-strings -Wcast-qual -Wfloat-equal
CFLAGS_STRICT_UNINITIALIZED =
JCFLAGS          = -g -Xlint:deprecation,unchecked,fallthrough,path,finally
