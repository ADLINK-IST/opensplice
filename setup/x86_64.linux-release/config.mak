# Set target context
PROC	     = x86_64
OS           = linux
OS_REV       =
SPECIAL      = RELEASE

ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/$(PROC).$(OS)$(OS_REV)-default.mak
else
include $(OSPL_HOME)/setup/$(PROC).$(OS)$(OS_REV)-default.mak
endif

# Compiler flags
CFLAGS_OPT       = -O3 -fno-strict-aliasing
CFLAGS_DEBUG     = -DNDEBUG
JCFLAGS          = -g -nowarn

# Set by configure when gcc version >= 4.7
ifeq ($(GCC_SUPPORTS_LTO),1)
CFLAGS_OPT += $(CFLAGS_LTO)
ifneq ($(AR_LTO),)
AR = $(AR_LTO)
endif
endif
