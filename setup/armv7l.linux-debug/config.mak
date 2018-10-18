ifdef OSPL_OUTER_HOME
include $(OSPL_OUTER_HOME)/setup/armv7l.linux-default.mak
else
include $(OSPL_HOME)/setup/armv7l.linux-default.mak
endif

SPECIAL      = DEBUG

# Compiler flags
CFLAGS_OPT       = -O0 -g
CFLAGS_DEBUG     = -DNDEBUG  -funwind-tables
CFLAGS_STRICT_UNINITIALIZED =
JCFLAGS          = -g

#Csc compiler flags
CSFLAGS_DEBUG    = -define:DEBUG\;TRACE -debug+ -debug:full
