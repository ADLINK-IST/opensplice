# -*- makefile -*-
# included by bld/$(SPLICE_TARGET)/makefile

.PHONY: everything everything1
link everything: everything1

# This makefile builds odlpp and $(DDS_CORE), which is assumed to
# expand to ddskernel, and then additionally empty/fake libraries with
# the old names
ifeq ($(SPLICE_TARGET),$(SPLICE_REAL_TARGET))
FAKE_LIBS_BASE := ddsutil ddsconf ddsconfparser dcpsgapi ddsos ddsosnet ddsuser ddsdatabase ddsserialization
endif

# Include directories are managed very inelegantly: pretty much
# everything under the sun is included in the path all the time,
# rather than following the dependencies that exist between the
# directories.  (While definitely possible using target-specific
# variables, IMHO it isn't worth the bother.)

# We need $(OS) and $(OSREV) before rules.mak can be included
ifeq "$(wildcard $(OSPL_HOME)/setup/$(SPLICE_TARGET))" ""
include $(OSPL_OUTER_HOME)/setup/$(SPLICE_TARGET)/config.mak
else
include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak
endif

ifeq "$(OSPL_OUTER_HOME)" ""
  # inner-ring build
  ABSTR_HOME    := $(OSPL_HOME)
  PROC_HOME     := $(OSPL_HOME)
else
  # check whether OS from outer or inner ring
  ifeq "$(wildcard $(OSPL_OUTER_HOME)/src/abstraction/os/$(OS)$(OS_REV))" ""
    ABSTR_HOME  := $(OSPL_HOME)
  else
    ABSTR_HOME  := $(OSPL_OUTER_HOME)
  endif
  # same for processor
  ifeq "$(wildcard $(OSPL_OUTER_HOME)/src/abstraction/pa/$(PROC))" ""
    PROC_HOME   := $(OSPL_HOME)
  else
    PROC_HOME   := $(OSPL_OUTER_HOME)
  endif
endif
ABSTR_HOME    := $(ABSTR_HOME)/src/abstraction
ABSTR_MODULES := os os-net
ST1_MODULES   := utilities database/database
ST1_MODULES   += database/serialization # in ST1 only because of IDLPP
ST2_MODULES   := configuration/config configuration/parser kernel user api/dcps/gapi api/dcps/common

#CONF2C is included on some host target split platforms only
#it requires the configuration parser in the DDS_HTS lib
ifeq ($(INCLUDE_TOOLS_CONF2C),yes)
ST1_MODULES   += configuration/config configuration/parser
endif

ABSTR_PATH    := $(ABSTR_MODULES:%=$(ABSTR_HOME)/%)
ifneq "$(ABSTR_HOME)" "$(OSPL_HOME)/src/abstraction"
  ABSTR_PATH  += $(ABSTR_MODULES:%=$(OSPL_HOME)/src/abstraction/%)
endif
PROC_PATH     := $(PROC_HOME)/src/abstraction/pa

# Include all the rules (and regulations as it were).  Sadly,
# rules.mak, target.mak insist on using CODE_DIR all the place,
# setting CINCS and vpath to all kinds of stuff that we now have to
# work around.
#
# Arguably, rules.mak ought to be enough, but target-only.mak is the
# one defining the "make_exec" and "make_dlib" macros, and rather than
# moving those to rules.mak and possibly breaking yet different
# things, just include target.mak.
CODE_DIR = NON_EXISTENT
include	$(OSPL_HOME)/setup/makefiles/target.mak
vpath
O = $(OBJ_POSTFIX)
X = $(EXEC_POSTFIX)

CPPFLAGS += -DOSPL_BUILD_CORE -DMODEL_kernelModule_IMPLEMENTATION
ifneq "$(OSPL_OUTER_HOME)" ""
  CPPFLAGS += -DUSE_ADV_MEM_MNG
endif
ifneq "$(findstring yes, $(INCLUDE_PLUGGABLE_REPORTING))" ""
  CPPFLAGS += -DINCLUDE_PLUGGABLE_REPORTING
endif
CFLAGS += $(SHCFLAGS) $(MTCFLAGS)
LDLIBS += $(LDLIBS_OS) $(LDLIBS_NW)
ifneq (,$(UT_TRACE_EXTENSION))
  LDLIBS += -lbfd -liberty
endif
ifneq (,$(findstring int5,$(SPLICE_TARGET)))
  LDLIBS += -lposix -lutil
endif

ODLPP_PATH := $(OSPL_HOME)/src/database/odlpp
ST1_PATH := $(ABSTR_PATH) $(ST1_MODULES:%=$(OSPL_HOME)/src/%)
ifneq "$(OSPL_OUTER_HOME)" ""
  ST1_PATH += $(OSPL_OUTER_HOME)/src/database/database
endif
vpath %.c $(addsuffix /code, $(ST1_PATH) $(ODLPP_PATH))
vpath %.l $(addsuffix /code, $(ST1_PATH) $(ODLPP_PATH))
vpath %.y $(addsuffix /code, $(ST1_PATH) $(ODLPP_PATH))
# Abstraction layer includes: the platform-specific bit mustn't have
# the "/include" suffix, so need some more special-casing, the
# wildcard is for catching all $(ABSTR_MODULES), i.e., os and os-net.
CINCS += $(addprefix -I, $(addsuffix /include, $(filter-out %/src/abstraction/%/$(OS)$(OS_REV), $(ST1_PATH))))

ifeq "$(UT_TRACE_EXTENSION)" ""
  ST1_C_FILES_TO_FILTER += ut_backtrace-symbols.c
endif
ST1_C_FILES := $(filter-out $(ST1_C_FILES_TO_FILTER), $(notdir $(wildcard $(addsuffix /code/*.c, $(ST1_PATH)))))
ST1_Y_FILES := q_parser.y
ST1_L_FILES := q_parser.l
ifeq ($(INCLUDE_TOOLS_CONF2C),yes)
ST1_Y_FILES += cfg_parser.y
ST1_L_FILES += cfg_scanner.l
endif
# Some files buried in the platform-specific files are intended to be
# included by the generic ones, but some (the os__*.c ones) are
# intended to be compiled directly.  So add the latter.
ST1_C_FILES += $(notdir $(wildcard $(ABSTR_MODULES:%=$(ABSTR_HOME)/%/$(OS)$(OS_REV)/code/os__*.c)))
vpath os__%.c $(ABSTR_MODULES:%=$(ABSTR_HOME)/%/$(OS)$(OS_REV)/code)
ST1_OBJECTS := $(ST1_C_FILES:%.c=%$O) $(ST1_Y_FILES:%.y=%$O)
ST1_DEPS := $(ST1_OBJECTS:%$O=%.d)

q_parser$O q_parser.d: q_parser.h

ODLPP_C_FILES := $(notdir $(wildcard $(ODLPP_PATH)/code/*.c))
ODLPP_Y_FILES := c_odlbase.y
ODLPP_L_FILES := c_odlbase.l
ODLPP_OBJECTS := $(ODLPP_C_FILES:%.c=%$O) $(ODLPP_Y_FILES:%.y=%$O)

zzzz:
	@echo 'ABSTR_PATH=$(ABSTR_PATH)'
	@echo 'PROC_PATH=$(PROC_PATH)'
	@echo 'ST1_PATH=$(ST1_PATH)'
	@echo 'ST2_PATH=$(ST2_PATH)'
	@echo 'CINCS=$(CINCS)'

ifeq ($(SPLICE_HOST),$(SPLICE_TARGET))

# When starting with a clean build directory and processing the
# include directives for dependency files, make doesn't yet know that
# the odlpp object file depenencies are dependent on the generation of
# the corresponding .d files.  Generating c_odlbase.d in turn is
# dependent on c_odlbase.h having been generated ...
ODLPP_DEPS    := $(ODLPP_OBJECTS:%$O=%.d)
$(ODLPP_DEPS): CINCS += -I$(OSPL_HOME)/src/database/odlpp/code 
c_odlbase$O c_odlbase.d: c_odlbase.h

odlpp$X: CINCS += -I$(OSPL_HOME)/src/database/odlpp/code
odlpp$X: | build_tools_stage
odlpp$X: $(ODLPP_OBJECTS) $(ST1_OBJECTS)
	$(make_exec)

$(SPLICE_EXEC_PATH)/odlpp$X: odlpp$X
	[ -d $(SPLICE_EXEC_PATH) ] || mkdir -p $(SPLICE_EXEC_PATH)
	$(make_exec_link)

build_tools_stage: $(SPLICE_LIBRARY_PATH)/$(DLIB_PREFIX)$(DDS_HTS)$(DLIB_POSTFIX)
	[ -d $(OSPL_HOME)/src/cpp/bld/$(SPLICE_HOST) ] || mkdir -p $(OSPL_HOME)/src/cpp/bld/$(SPLICE_HOST)
	$(MAKE) -C $(OSPL_HOME)/src/cpp/
	[ -d $(OSPL_HOME)/src/tools/cppgen/bld/$(SPLICE_HOST) ] || mkdir -p $(OSPL_HOME)/src/tools/cppgen/bld/$(SPLICE_HOST)
	$(MAKE) -C $(OSPL_HOME)/src/tools/cppgen/
	[ -d $(OSPL_HOME)/src/tools/idlpp/bld/$(SPLICE_HOST) ] || mkdir -p $(OSPL_HOME)/src/tools/idlpp/bld/$(SPLICE_HOST)
	$(MAKE) -C $(OSPL_HOME)/src/tools/idlpp/

$(DLIB_PREFIX)$(DDS_HTS)$(DLIB_POSTFIX): $(ST1_OBJECTS)
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), \
		$(findstring wince,$(SPLICE_TARGET))))
	$(LD_SO) $(SHLDFLAGS) $(LDFLAGS) $^ $(FILTERED_LDLIBS) -o $@
	ospl_wincmd mt -manifest $(addsuffix .manifest, $@) "-outputresource:$@;#2"
else
	$(LD_SO) $(SHLDFLAGS) $(LDFLAGS) $^ $(FILTERED_LDLIBS) -o $@
endif

$(SPLICE_LIBRARY_PATH)/$(DLIB_PREFIX)$(DDS_HTS)$(DLIB_POSTFIX): $(DLIB_PREFIX)$(DDS_HTS)$(DLIB_POSTFIX)
	[ -d $(SPLICE_LIBRARY_PATH) ] || mkdir -p $(SPLICE_LIBRARY_PATH)
	$(make_dlib_link)

everything1:: $(SPLICE_EXEC_PATH)/odlpp$X
endif

ST2_PATH := $(ST2_MODULES:%=$(OSPL_HOME)/src/%)
vpath %.c   $(addsuffix /code, $(ST2_PATH))
vpath %.l   $(addsuffix /code, $(ST2_PATH))
vpath %.y   $(addsuffix /code, $(ST2_PATH))
vpath %.odl $(addsuffix /code, $(ST2_PATH))

ST2_C_FILES := $(filter-out $(C_FILES_TO_FILTER), $(notdir $(wildcard $(addsuffix /code/*.c, $(ST2_PATH)))))
# Sadly, the ODL file name has nothing to do with the name of the
# generated .c and .h files, so we have to hard-code it:
ST2_C_FILES += kernelModule.c
# and then we use an ugly fake pattern rule (we know in this context
# the % expands to "k" in the sole case the rule applies) to explain
# to "make" that preprocessing a .odl file yields a .c and a .h file.
%ernelModule.STAMP: v_%ernel.odl $(SPLICE_EXEC_PATH)/odlpp$X
	for x in kernelModule.c kernelModule.h ; do [ -f $$x ] && mv $$x $$x.ORIG ; done ; true
	sh $(OSPL_HOME)/bin/sppodl $(SPPODL_FLAGS) $<
	for x in kernelModule.c kernelModule.h ; do [ -f $$x ] || { echo "Missing: $$x" >&2 ; exit 1 ; } done
	for x in kernelModule.c kernelModule.h ; do [ -f $$x.ORIG ] && diff -q $$x $$x.ORIG >/dev/null && mv $$x.ORIG $$x ; done ; true
	touch $@
	@if [ "$(SPLICE_HOST)" != "$(SPLICE_REAL_TARGET)" ] ; \
	then \
		[ -d $(OSPL_HOME)/src/osplcore/bld/$(SPLICE_REAL_TARGET) ] || \
			mkdir -p $(OSPL_HOME)/src/osplcore/bld/$(SPLICE_REAL_TARGET) ; \
		cp $(OSPL_HOME)/src/osplcore/bld/$(SPLICE_HOST)/kernelModule.c \
			$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_REAL_TARGET)/. ; \
		cp $(OSPL_HOME)/src/osplcore/bld/$(SPLICE_HOST)/kernelModule.h \
			$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_REAL_TARGET)/. ; \
		cp $(OSPL_HOME)/src/osplcore/bld/$(SPLICE_HOST)/kernelModule.STAMP \
			$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_REAL_TARGET)/. ; \
	fi
kernelModule.c kernelModule.h: | kernelModule.STAMP
ST2_Y_FILES := cfg_parser.y gapi_parser.y
ST2_L_FILES := cfg_scanner.l gapi_parser.l
ST2_OBJECTS := $(ST2_C_FILES:%.c=%$O) $(ST2_Y_FILES:%.y=%$O)
ST2_DEPS    := $(ST2_OBJECTS:%$O=%.d)

# QoS provider requires preprocessing IDL into .c/.h (although those
# .c files are actually included in qp_qosProvider.c rather than being
# compiled directly). Following the pattern for kernelModule
# here. This works as long as all that gets generated is SplType and
# SplLoad - but we can deal with that if and when it changes.
QP_IDL := dds_namedQosTypes.idl dds_dcps_builtintopics.idl dds_builtinTopics.idl
QP_IDL_H := $(QP_IDL:%.idl=%SplType.h)
QP_IDL_C := $(QP_IDL:%.idl=%SplLoad.c)
vpath %.idl $(OSPL_HOME)/etc/idl
%.idl.STAMP: %.idl
	for x in $*SplType.h $*SplLoad.c ; do [ -f $$x ] && mv $$x $$x.ORIG ; done ; true
	$(WINCMD) $(SPLICE_EXEC_PATH)/idlpp$X -l c -S -m SPLLOAD -m SPLTYPE $<
	for x in $*SplType.h $*SplLoad.c ; do [ -f $$x ] || { echo "Missing: $$x" >&2 ; exit 1 ; } done
	for x in $*SplType.h $*SplLoad.c ; do [ -f $$x.ORIG ] && diff -q $$x $$x.ORIG >/dev/null && mv $$x.ORIG $$x ; done ; true
	touch $@
	@if [ "$(SPLICE_HOST)" != "$(SPLICE_REAL_TARGET)" ] ; \
	then \
		[ -d $(OSPL_HOME)/src/osplcore/bld/$(SPLICE_REAL_TARGET) ] || \
			mkdir -p $(OSPL_HOME)/src/osplcore/bld/$(SPLICE_REAL_TARGET) ; \
		cp $(OSPL_HOME)/src/osplcore/bld/$(SPLICE_HOST)/*SplLoad.c \
			$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_REAL_TARGET)/. ; \
		cp $(OSPL_HOME)/src/osplcore/bld/$(SPLICE_HOST)/*SplType.h \
			$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_REAL_TARGET)/. ; \
		cp $(OSPL_HOME)/src/osplcore/bld/$(SPLICE_HOST)/$@ \
			$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_REAL_TARGET)/. ; \
	fi
$(QP_IDL_H) $(QP_IDL_C): $(QP_IDL:%.idl=%.idl.STAMP)

# The dependency generation for the kernel sources depends on
# v_kernel.odl having been preprocessed into kernelModule.h.
$(ST2_DEPS): kernelModule.h

# Similarly, that for the QoS provider depends on having run idlpp on
# the QoS provider IDL files
qp_defaultQos.d qp_qosProvider.d: $(QP_IDL_H)

# Again, parser/lexer dependencies:
cfg_parser.d cfg_parser$O: CINCS += -I$(OSPL_HOME)/src/configuration/parser/code
cfg_parser$O cfg_parser.d: cfg_scanner.h
gapi_parser.d gapi_parser$O: CINCS += -I$(OSPL_HOME)/src/api/dcps/gapi/code
gapi_parser$O gapi_parser.d: gapi_parser.h

CINCS += -I. $(addprefix -I, $(ST2_PATH:%=%/include))
# Some known uses of private header files outside their module (i.e.,
# bugs) have been accepted for years, and rather than fixing them now,
# simply add yet more include directories
CINCS += -I$(OSPL_HOME)/src/database/database/code

%.i: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CINCS) -E $< >$@

empty.c:
	echo '/* Conceptually empty file, but not an empty translation unit */' >$@
	echo '#include <stdio.h>' >>$@
	echo '#ifdef _WIN32' >>$@
	echo '__declspec(dllexport) ospl_empty_func(void) {}'  >>$@
	echo '#endif'  >>$@

ifeq "$(STATIC_LIB_ONLY)" "" ### DYNAMIC LIB

OSPLCORE := $(DLIB_PREFIX)$(DDS_CORE)$(DLIB_POSTFIX)
FAKE_LIBS := $(FAKE_LIBS_BASE:%=$(DLIB_PREFIX)%$(DLIB_POSTFIX))
ALL_LIBS = $(OSPLCORE) $(FAKE_LIBS)

$(ALL_LIBS): LDFLAGS += $(SHLDFLAGS)
$(ALL_LIBS): LDLIBS += $(SHLDLIBS)
$(FAKE_LIBS): LDLIBS += -l$(DDS_CORE)

$(OSPLCORE): $(ST1_OBJECTS) $(ST2_OBJECTS)
	$(make_dlib)

$(FAKE_LIBS): empty$O
	$(make_dlib)

$(SPLICE_LIBRARY_PATH)/%: %
	[ -d $(SPLICE_LIBRARY_PATH) ] || mkdir -p $(SPLICE_LIBRARY_PATH)
	$(make_dlib_link)

else ###

OSPLCORE := $(SLIB_PREFIX)$(DDS_CORE)$(SLIB_POSTFIX)
FAKE_LIBS := $(FAKE_LIBS_BASE:%=$(SLIB_PREFIX)%$(SLIB_POSTFIX))
ALL_LIBS = $(OSPLCORE) $(FAKE_LIBS)

$(OSPLCORE): $(ST1_OBJECTS) $(ST2_OBJECTS)
	$(make_slib)

$(FAKE_LIBS): empty$O
	$(make_slib)

$(SPLICE_LIBRARY_PATH)/%: %
	[ -d $(SPLICE_LIBRARY_PATH) ] || mkdir -p $(SPLICE_LIBRARY_PATH)
	$(make_slib_link)

endif ###

ifeq ($(SPLICE_TARGET),$(SPLICE_REAL_TARGET))
everything1:: $(ALL_LIBS:%=$(SPLICE_LIBRARY_PATH)/%)
endif

# Verify the parsers and scanners in this file match the source tree
ST1_Y_FILES_ACTUAL := $(notdir $(notdir $(wildcard $(addsuffix /code/*.y, $(ST1_PATH)))))
ST1_L_FILES_ACTUAL := $(notdir $(notdir $(wildcard $(addsuffix /code/*.l, $(ST1_PATH)))))
ifneq "$(sort $(ST1_Y_FILES_ACTUAL))" "$(sort $(ST1_Y_FILES))"
  $(error ST1_Y_FILES defined incorrectly: $(ST1_Y_FILES) vs $(ST1_Y_FILES_ACTUAL))
endif
ifneq "$(sort $(ST1_L_FILES_ACTUAL))" "$(sort $(ST1_L_FILES))"
  $(error ST1_L_FILES defined incorrectly: $(ST1_L_FILES) vs $(ST1_L_FILES_ACTUAL))
endif
ODLPP_Y_FILES_ACTUAL := $(notdir $(wildcard $(ODLPP_PATH)/code/*.y))
ODLPP_L_FILES_ACTUAL := $(notdir $(wildcard $(ODLPP_PATH)/code/*.l))
ifneq "$(sort $(ODLPP_Y_FILES_ACTUAL))" "$(sort $(ODLPP_Y_FILES))"
  $(error ODLPP_Y_FILES defined incorrectly: $(ODLPP_Y_FILES) vs $(ODLPP_Y_FILES_ACTUAL))
endif
ifneq "$(sort $(ODLPP_L_FILES_ACTUAL))" "$(sort $(ODLPP_L_FILES))"
  $(error ODLPP_L_FILES defined incorrectly: $(ODLPP_L_FILES) vs $(ODLPP_L_FILES_ACTUAL))
endif
ST2_Y_FILES_ACTUAL := $(notdir $(notdir $(wildcard $(addsuffix /code/*.y, $(ST2_PATH)))))
ST2_L_FILES_ACTUAL := $(notdir $(notdir $(wildcard $(addsuffix /code/*.l, $(ST2_PATH)))))
ifneq "$(sort $(ST2_Y_FILES_ACTUAL))" "$(sort $(ST2_Y_FILES))"
  $(error ST2_Y_FILES defined incorrectly: $(ST2_Y_FILES) vs $(ST2_Y_FILES_ACTUAL))
endif
ifneq "$(sort $(ST2_L_FILES_ACTUAL))" "$(sort $(ST2_L_FILES))"
  $(error ST2_L_FILES defined incorrectly: $(ST2_L_FILES) vs $(ST2_L_FILES_ACTUAL))
endif

ifeq "$(findstring zzzz, $(MAKECMDGOALS))" ""
-include $(ST1_DEPS) $(ODLPP_DEPS) $(ST2_DEPS)
endif
