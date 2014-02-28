# Set target context
PROC		 = x86_64
OS		     = linux
OS_REV		 =
SPECIAL		 = DEBUG

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/$(PROC).$(OS)$(OS_REV)-default.mak
else
include $(OSPL_HOME)/setup/$(PROC).$(OS)$(OS_REV)-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O0
# -DE_DEBUG is needed to enable debugging info for the SACPP apis
CFLAGS_DEBUG     = -g -D_TYPECHECK_ -DE_DEBUG
CFLAGS_STRICT_UNINITIALIZED =
JCFLAGS          = -g
