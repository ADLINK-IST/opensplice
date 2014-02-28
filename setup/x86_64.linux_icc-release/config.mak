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
CFLAGS_OPT       = -O3 -ipo
CFLAGS_DEBUG     = -DNDEBUG
JCFLAGS          = -g:none -nowarn

#Csc compiler flags
CSFLAGS_DEBUG    = -define:DEBUG\;TRACE -debug+ -debug:full

CC=icc
AR=xiar
CFLAGS_STRICT=
#LD_EXE=gcc -L$(INTEL_LIBIRC) -lirc
#CXX=g++ -L$(INTEL_LIBIRC) -lirc
CXX=icc
#LD_SO=gcc
FILTER=
