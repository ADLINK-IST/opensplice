# Set target context
PROC		 = x86
OS		     = win
OS_REV		 = 32
SPECIAL		 = DEBUG

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86.win32-default.mak
else
include $(OSPL_HOME)/setup/x86.win32-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -Od
# -D_DEBUG is needed to enable debugging info for the SACPP apis
CFLAGS_DEBUG     = -Z7 -D_TYPECHECK_ -D_DEBUG -MDd -DE_DEBUG
LDFLAGS          += -DEBUG -MANIFEST -NODEFAULTLIB:MSVCRT
#Java compiler flags
JCFLAGS          = -g
#Csc compiler flags
CSFLAGS_DEBUG    = -define:DEBUG\;TRACE -debug+ -debug:full