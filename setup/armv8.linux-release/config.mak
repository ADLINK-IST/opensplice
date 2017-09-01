ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/armv8.linux-default.mak
else
include $(OSPL_HOME)/setup/armv8.linux-default.mak
endif

SPECIAL = RELEASE

# Compiler flags
CFLAGS_OPT       = -O3 -fno-strict-aliasing
# -DE_DEBUG is needed to enable debugging info for the SACPP apis
CFLAGS_DEBUG     = -g -funwind-tables -DNDEBUG
CFLAGS_STRICT_UNINITIALIZED =
JCFLAGS          = -g

#Csc compiler flags
CSFLAGS_DEBUG    = 
