ifndef OSPL_OUTER_HOME
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_OS)
EXTRACTED_LIB = libddscore

include  $(OSPL_HOME)/setup/makefiles/target.mak

ifdef INCLUDE_PLUGGABLE_REPORTING
ifneq (,$(findstring yes,$(INCLUDE_PLUGGABLE_REPORTING)))
CPPFLAGS	+= -DINCLUDE_PLUGGABLE_REPORTING
CFLAGS      += -DINCLUDE_PLUGGABLE_REPORTING
endif
endif

CPPFLAGS	+= -DOSPL_BUILD_OS
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS    += -I$(OSPL_HOME)/src/abstraction/os/code
#CINCS    += -I$(OSPL_HOME)/src/database/database/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS	+= $(SHLDLIBS) $(LDLIBS_OS) $(LDLIBS_NW)

ifneq (,$(findstring int5,$(SPLICE_TARGET)))
LDLIBS     += -lposix -lutil
endif

# Deal with source files which only exist for some platforms, named os__*.c
LC_FILES        := $(notdir $(wildcard ../../$(OS)$(OS_REV)/code/os__*.c))
C_FILES		    += $(LC_FILES)
LOBJECTS        := $(LC_FILES:%.c=%$(OBJ_POSTFIX))
DEPENDENCIES    += $(LC_FILES:%.c=%.d)
vpath os__%.c ../../$(OS)$(OS_REV)/code/
OBJECTS+=$(LOBJECTS)
$(TARGET): $(LOBJECTS)

-include $(DEPENDENCIES)
else
.PHONY:
all compile link gcov qac clean analyse: 
	@$(MAKE) -C $(OSPL_OUTER_HOME)/src/abstraction/os $@
endif