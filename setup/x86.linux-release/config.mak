# Set target context
PROC		 = x86
OS		 = linux
OS_REV		 =
SPECIAL		 = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/x86.linux-default.mak
else
include $(OSPL_HOME)/setup/x86.linux-default.mak
endif

ifeq ($(FORCE_DEBUG_SYMBOLS), no)
# C only Compiler flags
CFLAGS += -std=c99

# Compiler flags
CFLAGS_OPT       = -O3 -g -fno-strict-aliasing $(CFLAGS_LTO)
CFLAGS_DEBUG     = -DNDEBUG
JCFLAGS          = -g

#Csc compiler flags
CSFLAGS_DEBUG    =
else
# Basically the release setting so it builds the core in release mode
# but also remove optimization and add debug symbols
CFLAGS_OPT       = -O0
CFLAGS_DEBUG     = -g -DNDEBUG
CFLAGS_STRICT_UNINITIALIZED =
JCFLAGS          = -g

#Csc compiler flags
CSFLAGS_DEBUG    =
endif
