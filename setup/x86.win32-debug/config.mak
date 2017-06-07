# Set target context
PROC		 = x86
OS           = win
OS_REV		 = 32
SPECIAL		 = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86.win32-default.mak
else
include $(OSPL_HOME)/setup/x86.win32-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -Z7 -O2 -DNDEBUG -MD
CFLAGS_DEBUG     =
LDFLAGS          += -DEBUG -MANIFEST -NODEFAULTLIB:MSVCRTD
JCFLAGS          = -g
#Csc compiler flags
CSFLAGS_DEBUG    =