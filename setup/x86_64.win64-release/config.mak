# Set target context
PROC		 = x86_64
OS               = win
OS_REV		 = 32
SPECIAL		 = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86_64.win64-default.mak
else
include $(OSPL_HOME)/setup/x86_64.win64-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O2 -DNDEBUG -MD
CFLAGS_DEBUG     = -Z7
LDFLAGS          += -DEBUG -MANIFEST -NODEFAULTLIB:MSVCRTD
JCFLAGS          = -g
#Csc compiler flags
CSFLAGS_DEBUG    =
