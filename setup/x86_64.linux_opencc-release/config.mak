# Set target context
PROC	     = x86_64
OS           = linux
OS_REV       =
SPECIAL      = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/$(PROC).$(OS)$(OS_REV)_opencc-default.mak
else
include $(OSPL_HOME)/setup/$(PROC).$(OS)$(OS_REV)_opencc-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O3 -fno-math-errno -ffast-math -fno-strict-aliasing
CFLAGS_OPT_C     = -O3 -OPT:Ofast -OPT:malloc_algorithm=0 -fno-math-errno -ffast-math -fno-strict-aliasing -march=anyx86
CFLAGS_DEBUG     = -DNDEBUG
JCFLAGS          = -g:none -nowarn
