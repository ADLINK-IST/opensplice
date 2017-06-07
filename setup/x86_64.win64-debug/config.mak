# Set target context
PROC		 = x86_64
OS               = win
OS_REV		 = 32
SPECIAL		 = DEBUG

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86_64.win64-default.mak
else
include $(OSPL_HOME)/setup/x86_64.win64-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -Od -DNDEBUG
CFLAGS_DEBUG     = -Z7 -MDd -DE_DEBUG
LDFLAGS          += -DEBUG -MANIFEST -NODEFAULTLIB:MSVCRT
#Java compiler flags
JCFLAGS          = -g
#Csc compiler flags
CSFLAGS_DEBUG    = -define:DEBUG\;TRACE -debug+ -debug:full