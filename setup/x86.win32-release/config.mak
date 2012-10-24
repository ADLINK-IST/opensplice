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
CFLAGS_OPT       = -O2 -DNDEBUG -MD
CFLAGS_DEBUG     = 
LDFLAGS          += -MANIFEST -NODEFAULTLIB:MSVCRTD
JCFLAGS          = -g:none
#Csc compiler flags
CSFLAGS_DEBUG    = 