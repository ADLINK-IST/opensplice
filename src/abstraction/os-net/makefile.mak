ifndef OSPL_OUTER_HOME
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_OS_NET)

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_OSNET
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS		+= -I$(OSPL_HOME)/src/database/database/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS	+= $(SHLDLIBS) $(LDLIBS_OS) $(LDLIBS_NW)
LDLIBS	+= -lddsos

LC_FILES := $(notdir $(wildcard ../../$(OS)$(OS_REV)/code/os__*.c))
LOBJECTS := $(LC_FILES:%.c=%$(OBJ_POSTFIX))

$(LC_FILES:%.c=%$(OBJ_POSTFIX)): ../../$(OS)$(OS_REV)/code/$(LC_FILES) 
	@echo $(CC) $(CFLAGS) $(CINCS) -c $<
	@filter_gcc $(CC) $(CFLAGS) $(CINCS) -c $<

$(LC_FILES:%.c=%.d): $(LC_FILES)
	$(CPP) $(MAKEDEPFLAGS) $(CINCS) $< >$@

$(TARGET): $(LOBJECTS)

-include $(DEPENDENCIES)
else
.PHONY:
all compile link gcov qac clean analyse: 
	@$(MAKE) -C $(OSPL_OUTER_HOME)/src/abstraction/os-net $@
endif
