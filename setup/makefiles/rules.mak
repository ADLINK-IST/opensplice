ifneq (,$(wildcard $(OSPL_HOME)/setup/$(SPLICE_TARGET)))
include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak
else
include $(OSPL_OUTER_HOME)/setup/$(SPLICE_TARGET)/config.mak
endif

# This makefile defined the platform and component independent make rules.
CLASS_DIR ?=bld/$(SPLICE_TARGET)
JCODE_DIR ?=code
JCODE_PATH ?= $(JCODE_DIR)
JCFLAGS    +=-source 1.6 -target 1.6 -sourcepath '$(JCODE_PATH)'

MANIFEST_TARGET := manifest/$(SPLICE_TARGET)
MANIFEST     = $(MANIFEST_TARGET)/manifest.mf
MANIFEST_TMP = $(MANIFEST_TARGET)/manifest.tmp

# If JAR_MODULE is not defined, assign something to prevent warnings in make
# output.
JAR_MODULE ?= foo

JAR_TARGET =$(JAR_LOCATION)/jar/$(SPLICE_TARGET)
JAR_FILE   =$(JAR_TARGET)/$(JAR_MODULE)

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
JSEPARATOR := :
JNORMALIZE = $(1)
else
JSEPARATOR := ;
JNORMALIZE = $(shell cygpath -m $(1))
endif

ifdef JAVA_MAIN_CLASS
MANIFEST_MAIN=Main-Class: $(JAVA_MAIN_CLASS)
endif

ifdef JAVA_INC
  ifeq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
    # not building on Windows
    ifeq (,$(findstring testsuite,$(CURDIR)))
      # not building a test: strip paths
      # $(notdir) doesn't cope with spaces in pathnames. NB this workaround doesn't cope with ? in pathnames.
      JAVA_INC2=$(subst $(empty) ,?,$(JAVA_INC))
      MANIFEST_CLASSPATH=Class-Path: $(filter %.jar,$(subst ?, ,$(notdir $(subst :, ,$(JAVA_INC2)))))
    else
      # building a test: don't strip paths
      JAVA_INC2=$(subst $(empty) ,\n  ,$(foreach elem,$(filter %.jar,$(subst :, ,$(subst $(empty) ,?,$(JAVA_INC)))),file:///$(elem)))
      MANIFEST_CLASSPATH=Class-Path: $(subst ?, ,$(JAVA_INC2))
    endif
  else
    # building on Windows
    ifeq (,$(findstring testsuite,$(CURDIR)))
      # not building a test: strip paths
      JAVA_INC2=$(subst $(empty) ,?,$(JAVA_INC))
      MANIFEST_CLASSPATH=Class-Path: $(filter %.jar,$(subst ?, ,$(notdir $(subst :, ,$(JAVA_INC2)))))
    else
      # building a test: convert and keep paths
	  JAVA_INC2=$(subst ;, ,$(subst $(empty) ,?,$(shell cygpath -p -m "$(JAVA_INC)")))
	  MANIFEST_CLASSPATH=Class-Path: $(subst ?, ,$(subst $(empty) ,\n  ,$(filter %.jar, $(JAVA_INC2:%=file:///%))))
    endif
  endif
endif

ifdef JACORB_HOME
CP_JACORB_IDL=-classpath "$(JACORB_HOME)/lib/idl.jar:$(JACORB_HOME)/lib/endorsed/logkit.jar"

ifeq "$(JAVA_ORB)" "JACORB"
JAVA_ORB_INC=$(JACORB_INC)
JCFLAGS_ORB=$(JCFLAGS_JACORB)
endif

endif

JCC_ARGS=$(JCFLAGS_COMPAT) $(JCFLAGS) $(JCFLAGS_ORB) $(JCFLAGS_EXTRA) -classpath "$(JAVA_ORB_INC):$(JAVA_SYSTEM_INC):$(JAVA_INC)" -d $(CLASS_DIR)

ifeq "$(origin VERBOSE)" "undefined"
	AT_SIGN=@
else
	AT_SIGN=
endif

$(CLASS_DIR):
	$(AT_SIGN)mkdir -p $@

$(CLASS_DIR)/.STAMP:
	$(AT_SIGN)mkdir -p $(@D)
	$(AT_SIGN)touch $@

LOCAL_CLASS_DIR	=$(CLASS_DIR)/$(PACKAGE_DIR)

ifdef OSPL_INNER_REV
   ifneq (,$(OSPL_INNER_REV))
      OSPL_REV_FLAGS  = -DOSPL_INNER_REV=$(OSPL_INNER_REV)
   endif
endif

ifdef OSPL_OUTER_REV
   ifneq (,$(OSPL_OUTER_REV))
      OSPL_REV_FLAGS  += -DOSPL_OUTER_REV=$(OSPL_OUTER_REV)
   endif
endif

ifdef SPLICE_HOST_FULL
   ifneq (,$(SPLICE_HOST_FULL))
      OSPL_HOSTTARGET_FLAGS += -DOSPL_HOST=\"$(SPLICE_HOST_FULL)\"
   endif
endif

ifdef SPLICE_TARGET_FULL
   ifneq (,$(SPLICE_TARGET_FULL))
      OSPL_HOSTTARGET_FLAGS += -DOSPL_TARGET=$(SPLICE_TARGET_FULL)
   endif
endif

ifeq (solaris,$(OS))
   OSREV_FLAGS += -DOS_SOLARIS_VER=$(OS_REV)
   OS_REV_SUFFIX=
else
   OS_REV_SUFFIX=$(OS_REV)
endif

CPPFLAGS+=-DOSPL_VERSION=$(PACKAGE_VERSION) $(OSPL_REV_FLAGS) $(OSPL_HOSTTARGET_FLAGS) $(OSREV_FLAGS)

.PRECIOUS: %.c %.h

(%.o): %.o
	$(AR) r $@ $<

ECHO_COMMAND=echo

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
ifeq (,$(findstring studio,$(SPLICE_HOST)))
%.d: %.c
	@$(ECHO_COMMAND) DEP $<
	$(AT_SIGN)$(CPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CINCS) $< >$@
else
%.d: %.c
	@$(ECHO_COMMAND) DEP $<
	$(AT_SIGN)$(CPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CINCS) $< | sed -e 's/\([^:\\]\) /\1\\ /g' >$@
endif
else
%.d: %.c
	@$(ECHO_COMMAND) DEP $<
	$(AT_SIGN)$(CPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CINCS) $< | grep "^#line.*\\\\ospl[io]\\\\" | cut -d '"' -f 2 | sort -u | sed -e 's@\([A-Za-z]\)\:@ /cygdrive/\1@' -e 's@\\\\@/@g' -e '$$!s@$$@ \\@' -e '1s@^@$*$(OBJ_POSTFIX): @' >$@
endif

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
ifeq (,$(findstring studio,$(SPLICE_HOST)))
%.d: %.cpp
	@$(ECHO_COMMAND) DEP $<
	$(AT_SIGN)$(GCPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -MT$(@:%.d=%$(OBJ_POSTFIX)) $< > $@
else
%.d: %.cpp
	@$(ECHO_COMMAND) DEP $<
	$(AT_SIGN)$(GCPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) $< | sed -e 's/\([^:\\]\) /\1\\ /g' >$@
endif
else
%.d: %.cpp
	@$(ECHO_COMMAND) DEP $<
	$(AT_SIGN)$(GCPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) $< | grep "^#line.*\\\\ospl[io]\\\\" | cut -d '"' -f 2 | sort -u | sed -e 's@\([A-Za-z]\)\:@ /cygdrive/\1@' -e 's@\\\\@/@g' -e '$$!s@$$@ \\@' -e '1s@^@$*$(OBJ_POSTFIX): @' >$@
endif

# $(abspath x) is extremely useful as debuggers and profilers
# typically can't find the sources using the relative path, but
# potentially can using the absolute path. In combination with the
# path prefix mapping supported by gdb and lldb, using an absolute
# path should make life easier.
#
# Unfortunately $(abspath x) was introduced in version 3.81 while we
# still use make 3.80 in various places. If $(abspath /) expands to
# an empty string, obviously something is wrong with $(abspath x), and
# by far the most likely is that it is not supported. We then fall
# back to using the relative paths we have been using until now.
ifeq "$(abspath /)"""
%$(OBJ_POSTFIX): %.c
	@$(ECHO_COMMAND) $(CC) $<
	$(AT_SIGN)$(CC) $(CPPFLAGS) $(CFLAGS) $(CINCS) -o $@ -c $<

%$(OBJ_POSTFIX): %.cpp
	@$(ECHO_COMMAND) $(CXX) $<
	$(AT_SIGN)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -o $@ -c $<

%$(OBJ_POSTFIX): %.cc
	@$(ECHO_COMMAND) $(CXX) $<
	$(AT_SIGN)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -o $@ -c $<
else
%$(OBJ_POSTFIX): %.c
	@$(ECHO_COMMAND) $(CC) $<
	$(AT_SIGN)$(CC) $(CPPFLAGS) $(CFLAGS) $(CINCS) -o $@ -c $(abspath $<)

%$(OBJ_POSTFIX): %.cpp
	@$(ECHO_COMMAND) $(CXX) $<
	$(AT_SIGN)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -o $@ -c $(abspath $<)

%$(OBJ_POSTFIX): %.cc
	@$(ECHO_COMMAND) $(CXX) $<
	$(AT_SIGN)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -o $@  -c $(abspath $<)
endif

%.c: %.y
	$(YACC) $< -o $@

%.h: %.l
	$(LEX) -t  -P$(basename $@)_yy $< | perl -np -e 's/\berrno\s*=\s*(\w+)/os_setErrno(\1)/g; s/\berrno\b(?![\.])/os_getErrno()/g;' > $@

$(CLASS_DIR)/%.class: $(JCODE_DIR)/%.java
	$(JCC) $(JCC_ARGS) $(dir $<)*.java

CODE_DIR	?= ../../code

vpath %.c		$(CODE_DIR)
vpath %.cpp		$(CODE_DIR)
vpath %.y		$(CODE_DIR)
vpath %.l		$(CODE_DIR)
vpath %.odl		$(CODE_DIR)
#vpath %.class 	$(LOCAL_CLASS_DIR)
vpath %.idl		$(CODE_DIR)
vpath %.proto	$(CODE_DIR)
CINCS		 = -I.
CINCS		+= -I../../include
CINCS		+= -I$(CODE_DIR)

ifndef OSPL_OUTER_HOME
CINCS		+= -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os-net/$(OS)$(OS_REV_SUFFIX)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os-net/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV_SUFFIX)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/include
#Added to allow inclusion of source from one abstraction layer into another
CINCS		+= -I$(OSPL_HOME)/src/abstraction
else
CINCS		+= -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction/os-net/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os-net/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os-net/$(OS)$(OS_REV_SUFFIX)
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction/os-net/$(OS)$(OS_REV_SUFFIX)
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction/os/$(OS)$(OS_REV_SUFFIX)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV_SUFFIX)
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction/os/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/include
#Added to allow inclusion of source from one abstraction layer into another
#like vxworks6.6 into some of the source files of vxworks6.9 etc
CINCS		+= -I$(OSPL_HOME)/src/abstraction
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction
endif

CXXINCS	+= -I.
CXXINCS	+= -I../../include
CXXINCS	+= -I$(CODE_DIR)

ifndef OSPL_OUTER_HOME
CXXINCS	+= -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
CXXINCS	+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV_SUFFIX)
CXXINCS += -I$(OSPL_HOME)/src/abstraction/os/include
else
CXXINCS	+= -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
CXXINCS += -I$(OSPL_OUTER_HOME)/src/abstraction/os/include
CXXINCS += -I$(OSPL_HOME)/src/abstraction/os/include
CXXINCS	+= -I$(OSPL_OUTER_HOME)/src/abstraction/os/$(OS)$(OS_REV_SUFFIX)
CXXINCS	+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV_SUFFIX)
endif

C_FILES		?= $(filter-out $(C_FILES_TO_FILTER), $(notdir $(wildcard $(CODE_DIR)/*.c)))
CPP_FILES	?= $(filter-out $(CPP_FILES_TO_FILTER), $(notdir $(wildcard $(CODE_DIR)/*.cpp)))
Y_FILES		:= $(notdir $(wildcard $(CODE_DIR)/*.y))
L_FILES		:= $(notdir $(wildcard $(CODE_DIR)/*.l))
ODL_FILES	?= $(notdir $(wildcard $(CODE_DIR)/*.odl))
GCOV_FILES	:= $(notdir $(wildcard *.bb))
JAVA_FILES	?= $(wildcard $(addsuffix /*.java,$(addprefix $(JCODE_DIR)/,$(JPACKAGES))))
CLASS_FILES  = $(subst .java,.class,$(subst $(JCODE_DIR),$(CLASS_DIR),$(JAVA_FILES)))
CS_FILES	 = $(wildcard $(addsuffix /*.cs,$(addprefix $(CODE_DIR)/,$(CS_NAMESPCS)))) $(IDL_CS) $(ODL_CS)

ODL_H		:= $(addsuffix .h,$(ODL_MODULES))
ODL_C		:= $(addsuffix .c,$(ODL_MODULES))
ODL_O		:= $(addsuffix $(OBJ_POSTFIX),$(ODL_MODULES))

ifndef OBJECTS
OBJECTS := $(EXTRAOBJECTS) $(Y_FILES:%.y=%$(OBJ_POSTFIX)) $(ODL_O) $(IDL_O) $(PROTO_O) $(C_FILES:%.c=%$(OBJ_POSTFIX)) $(CPP_FILES:%.cpp=%$(OBJ_POSTFIX))
endif

DEPENDENCIES	:= $(C_FILES:%.c=%.d) $(CPP_FILES:%.cpp=%.d) $(Y_FILES:%.y=%.d) $(ODL_C:%.c=%.d)
EXECUTABLES     := $(basename $(OBJECTS))
H_FILES		:= $(L_FILES:%.l=%.h)

$(H_FILES): $(L_FILES) #$(ODL_FILES) $(IDL_FILES)
#$(OBJECTS): $(C_FILES) $(CPP_FILES) $(Y_FILES:%.y=%.c) $(ODL_C)
#$(DEPENDENCIES): $(C_FILES) $(CPP_FILES) $(Y_FILES:%.y=%.c) $(H_FILES) $(ODL_H)
$(DEPENDENCIES): $(H_FILES) $(ODL_H) $(IDL_H) $(JNI_H) $(PROTO_H)

$(ODL_H): $(ODL_FILES)
	sh $(OSPL_HOME)/bin/sppodl $(SPPODL_FLAGS) $<

$(ODL_C): $(ODL_FILES)
	sh $(OSPL_HOME)/bin/sppodl $(SPPODL_FLAGS) $<

$(ODL_CS): $(ODL_FILES)
	sh $(OSPL_HOME)/bin/sppodl $(SPPODL_FLAGS) $<

.PHONY: jar
jar: $(JAR_FILE)

# Testsuite has so many makefiles that don't work with a rule that
# generates a jar file straight from the java files and would gain
# nothing from it anyway that it is just not cost-effective enough to
# fix them all right now. This'll probably always be the case, but so
# be it ...
ifneq "$(findstring /testsuite/, $(PWD))" ""
$(JAR_FILE): $(JAR_DEPENDENCIES) $(CLASS_DIR)/.STAMP $(CLASS_FILES) $(JAR_TARGET)/.STAMP $(MANIFEST)
	$(JAR) cmf $(MANIFEST) $(call JNORMALIZE,$(JAR_FILE)) -C $(CLASS_DIR) .
else
$(JAR_FILE): $(JAR_DEPENDENCIES) $(CLASS_DIR)/.STAMP $(JAVA_FILES) $(JAR_TARGET)/.STAMP $(MANIFEST)
	@echo $(sort $(call JNORMALIZE,$(filter %.java, $^))) > $(CLASS_DIR)/java_files.txt
	$(AT_SIGN)$(JCC) $(JCC_ARGS) @$(CLASS_DIR)/java_files.txt
	$(JAR) cmf $(MANIFEST) $(call JNORMALIZE,$(JAR_FILE)) -C $(CLASS_DIR) .
endif

CURRENT_JAVA_DDS_VERSION = "Implementation-Version:\ $(PACKAGE_VERSION)"

ifdef OSPL_INNER_REV
ifneq (,$(OSPL_INNER_REV))
CURRENT_JAVA_DDS_VERSION_1 = $(shell echo "$(CURRENT_JAVA_DDS_VERSION), build $(OSPL_INNER_REV)")
endif
endif

ifdef OSPL_OUTER_REV
ifneq (,$(OSPL_OUTER_REV))
CURRENT_JAVA_DDS_VERSION_GIT = $(shell echo "$(CURRENT_JAVA_DDS_VERSION_1)/$(OSPL_OUTER_REV)")
endif
else
CURRENT_JAVA_DDS_VERSION_GIT = $(shell echo "$(CURRENT_JAVA_DDS_VERSION), non-ADLINK build")
endif

$(MANIFEST_TARGET):
	@mkdir -p $@

# If a manifest template is given, use that and don't start empty.
# Be sure that the template has the proper versions. Also remove
# possible rlm references from the manifest when it isn't needed.
# Add some global information at the end.
$(MANIFEST): | $(MANIFEST_TARGET)
	@rm -f $(MANIFEST)
	@rm -f $(MANIFEST_TMP)
ifdef MANIFEST_TEMPLATE
	@cp $(MANIFEST_TEMPLATE) $(MANIFEST_TMP)
ifeq (,$(findstring darwin,$(SPLICE_HOST)))
	@sed "s@<ospl-version>@$(BASE_VERSION)@" -i $(MANIFEST_TMP)
	@sed "s@<rlm-version>@$(RLM_VERSION)@" -i $(MANIFEST_TMP)
else
	@sed -e "s@<ospl-version>@$(BASE_VERSION)@" -i "" $(MANIFEST_TMP)
	@sed -e "s@<rlm-version>@$(RLM_VERSION)@" -i "" $(MANIFEST_TMP)
endif
ifneq "$(DDS_USE_LICENSE)" "yes"
ifeq (,$(findstring darwin,$(SPLICE_HOST)))
	@sed "\@^ com.reprisesoftware.rlm@d" -i $(MANIFEST_TMP)
	@sed "s@,*com.reprisesoftware.rlm,*@@" -i $(MANIFEST_TMP)
else
	@sed -e "\@^ com.reprisesoftware.rlm@d" -i "" $(MANIFEST_TMP)
	@sed -e "s@,*com.reprisesoftware.rlm,*@@" -i "" $(MANIFEST_TMP)
endif
endif
else
	@touch $(MANIFEST_TMP)
endif
	@echo "$(CURRENT_JAVA_DDS_VERSION_GIT)" >> $(MANIFEST_TMP)
ifdef MANIFEST_CLASSPATH
ifeq (solaris,$(OS))
	@/bin/echo "$(MANIFEST_CLASSPATH)" >> $(MANIFEST_TMP)
else
  ifeq "$(findstring darwin, $(SPLICE_HOST))" ""
	@/bin/echo -e "$(MANIFEST_CLASSPATH)" >> $(MANIFEST_TMP)
  else
	bash -c 'echo -e "$(MANIFEST_CLASSPATH)" >> $(MANIFEST_TMP)'
  endif
endif
endif
	@echo "$(MANIFEST_MAIN)" >> $(MANIFEST_TMP)
	@mv $(MANIFEST_TMP) $(MANIFEST)

$(JAR_TARGET):
	$(AT_SIGN)mkdir -p $@

$(JAR_TARGET)/.STAMP:
	$(AT_SIGN)mkdir -p $(@D)
	$(AT_SIGN)touch $@
