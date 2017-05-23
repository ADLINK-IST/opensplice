#
# included by bld/$(SPLICE_TARGET)/makefile
TARGET_DLIB := lzf-ospl
CODE_DIR    := ../../src
C_FILES     := lzf_c.c lzf_d.c

include $(OSPL_HOME)/setup/makefiles/rules.mak

#current CFLAGS environment not correctly setup for compiling external sources.
#CFLAGS	 := $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(SHCFLAGS) -Wall
#ifdef CPPFLAGS_ENV
#CPPFLAGS := $(CPPFLAGS_ENV)
#endif
CFLAGS   += $(SHCFLAGS) $(CFLAGS_PERMISSIVE)
CINCS	 :=

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)

include $(OSPL_HOME)/setup/makefiles/target-only.mak

zzzz:
	@echo CFLAGS=$(CFLAGS)
	@echo CPPFLAGS=$(CPPFLAGS)

-include $(DEPENDENCIES)
