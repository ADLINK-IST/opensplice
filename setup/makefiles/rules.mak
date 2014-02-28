ifneq (,$(wildcard $(OSPL_HOME)/setup/$(SPLICE_TARGET)))
include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak
else
include $(OSPL_OUTER_HOME)/setup/$(SPLICE_TARGET)/config.mak
endif

# This makefile defined the platform and component independent make rules.
CLASS_DIR  =bld/$(SPLICE_TARGET)
JCODE_DIR ?=code
JCODE_PATH ?= $(JCODE_DIR)
JCFLAGS    +=-sourcepath '$(JCODE_PATH)'
MANIFEST   =manifest/$(SPLICE_TARGET)/manifest.mf

# If JAR_MODULE is not defined, assign something to prevent warnings in make
# output.
JAR_MODULE ?= foo

JAR_TARGET =$(JAR_LOCATION)/jar/$(SPLICE_TARGET)
JAR_FILE   =$(JAR_TARGET)/$(JAR_MODULE)

ifdef JAVA_MAIN_CLASS
MANIFEST_MAIN=Main-Class: $(JAVA_MAIN_CLASS)
endif

ifdef JAVA_INC
# $(notdir) doesn't cope with spaces in pathnames. NB this workaround doesn't
# cope with ? in pathnames.
JAVA_INC2 = $(subst $(empty) ,?,$(JAVA_INC))

ifeq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
MANIFEST_CLASSPATH=Class-Path: $(filter %.jar,$(subst ?, ,$(notdir $(subst :, ,$(JAVA_INC2)))))
else
MANIFEST_CLASSPATH=Class-Path: $(filter %.jar,$(subst ?, ,$(notdir $(subst \,/,$(subst ;, ,$(subst :, ,$(JAVA_INC2)))))))
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
	mkdir -p $(CLASS_DIR)

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
      OSPL_HOSTTARGET_FLAGS += -DOSPL_TARGET=\"$(SPLICE_TARGET_FULL)\"
   endif
endif

CPPFLAGS+=-DOSPL_VERSION=$(PACKAGE_VERSION) $(OSPL_REV_FLAGS) $(OSPL_HOSTTARGET_FLAGS)

.PRECIOUS: %.c %.h

(%.o): %.o
	$(AR) r $@ $<

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
ifeq (,$(findstring studio,$(SPLICE_HOST)))
%.d: %.c
	@echo DEP $<
	$(AT_SIGN)$(CPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CINCS) $< >$@
else
%.d: %.c
	@echo DEP $<
	$(AT_SIGN)$(CPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CINCS) $< | sed -e 's/\([^:\\]\) /\1\\ /g' >$@
endif
else
%.d: %.c
	@echo DEP $<
	$(AT_SIGN)$(CPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CINCS) $< | grep "^#line.*\\\\ospl[io]\\\\" | cut -d '"' -f 2 | sort -u | sed -e 's@\([A-Za-z]\)\:@ /cygdrive/\1@' -e 's@\\\\@/@g' -e '$$!s@$$@ \\@' -e '1s@^@$*$(OBJ_POSTFIX): @' >$@
endif

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
ifeq (,$(findstring studio,$(SPLICE_HOST)))
%.d: %.cpp
	@echo DEP $<
	$(AT_SIGN)$(GCPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CXXINCS) $< >$@
else
%.d: %.cpp
	@echo DEP $<
	$(AT_SIGN)$(GCPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CXXINCS) $< | sed -e 's/\([^:\\]\) /\1\\ /g' >$@
endif
else
%.d: %.cpp
	@echo DEP $<
	$(AT_SIGN)$(GCPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CXXINCS) $< | grep "^#line.*\\\\ospl[io]\\\\" | cut -d '"' -f 2 | sort -u | sed -e 's@\([A-Za-z]\)\:@ /cygdrive/\1@' -e 's@\\\\@/@g' -e '$$!s@$$@ \\@' -e '1s@^@$*$(OBJ_POSTFIX): @' >$@
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
	@echo $(CC) $<
	$(AT_SIGN)$(FILTER) $(CC) $(CPPFLAGS) $(CFLAGS) $(CINCS) -c $<

%$(OBJ_POSTFIX): %.cpp
	@echo $(CXX) $<
	$(AT_SIGN)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -c $<

%$(OBJ_POSTFIX): %.cc
	@echo $(CXX) $<
	$(AT_SIGN)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -c $<
else
%$(OBJ_POSTFIX): %.c
	@echo $(CC) $<
	$(AT_SIGN)$(FILTER) $(CC) $(CPPFLAGS) $(CFLAGS) $(CINCS) -c $(abspath $<)

%$(OBJ_POSTFIX): %.cpp
	@echo $(CXX) $<
	$(AT_SIGN)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -c $(abspath $<)

%$(OBJ_POSTFIX): %.cc
	@echo $(CXX) $<
	$(AT_SIGN)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -c $(abspath $<)
endif

%.c: %.y
	$(YACC) $< -o $@

%.h: %.l
	$(LEX) -t  -P$(basename $@)_yy $< > $@

%.c.met: %.c
	$(QAC) $(CPFLAGS) $(CINCS) $<

%.c.gcov: %.bb
	$(GCOV) -b -f $< > $@.sum

$(CLASS_DIR)/%.class: $(JCODE_DIR)/%.java
	$(JCC) $(JCC_ARGS) $(dir $<)*.java

CODE_DIR	?= ../../code

vpath %.c		$(CODE_DIR)
vpath %.cpp		$(CODE_DIR)
vpath %.y		$(CODE_DIR)
vpath %.l		$(CODE_DIR)
vpath %.odl		$(CODE_DIR)
#vpath %.class 	$(LOCAL_CLASS_DIR)
vpath %.idl	$(CODE_DIR)
CINCS		 = -I.
CINCS		+= -I../../include
CINCS		+= -I$(CODE_DIR)

ifndef OSPL_OUTER_HOME
CINCS		+= -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os-net/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os-net/$(OS)$(OS_REV)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/pa/$(PROC_CORE)
#Added to allow inclusion of source from one abstraction layer into another
CINCS		+= -I$(OSPL_HOME)/src/abstraction
else
CINCS		+= -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction/os-net/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os-net/include
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction/os-net/$(OS)$(OS_REV)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os-net/$(OS)$(OS_REV)
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction/os/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/include
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction/os/$(OS)$(OS_REV)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction/pa/$(PROC_CORE)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/pa/$(PROC_CORE)
#Added to allow inclusion of source from one abstraction layer into another
#like vxworks6.6 into some of the source files of vxworks6.9 etc
CINCS		+= -I$(OSPL_HOME)/src/abstraction
CINCS		+= -I$(OSPL_OUTER_HOME)/src/abstraction
endif

CXXINCS	 = -I.
CXXINCS	+= -I../../include
CXXINCS	+= -I$(CODE_DIR)

ifndef OSPL_OUTER_HOME
CXXINCS	+= -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
CXXINCS += -I$(OSPL_HOME)/src/abstraction/os/include
CXXINCS	+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
else
CXXINCS	+= -I$(OSPL_HOME)/src/osplcore/bld/$(SPLICE_TARGET)
CXXINCS += -I$(OSPL_OUTER_HOME)/src/abstraction/os/include
CXXINCS += -I$(OSPL_HOME)/src/abstraction/os/include
CXXINCS	+= -I$(OSPL_OUTER_HOME)/src/abstraction/os/$(OS)$(OS_REV)
CXXINCS	+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
endif

C_FILES		?= $(filter-out $(C_FILES_TO_FILTER), $(notdir $(wildcard $(CODE_DIR)/*.c)))
CPP_FILES	?= $(filter-out $(CPP_FILES_TO_FILTER), $(notdir $(wildcard $(CODE_DIR)/*.cpp)))
Y_FILES		:= $(notdir $(wildcard $(CODE_DIR)/*.y))
L_FILES		:= $(notdir $(wildcard $(CODE_DIR)/*.l))
ODL_FILES	= $(notdir $(wildcard $(CODE_DIR)/*.odl))
GCOV_FILES	:= $(notdir $(wildcard *.bb))
JAVA_FILES	?= $(wildcard $(addsuffix /*.java,$(addprefix $(JCODE_DIR)/,$(JPACKAGES))))
CLASS_FILES = $(subst .java,.class,$(subst $(JCODE_DIR),$(CLASS_DIR),$(JAVA_FILES)))
CS_FILES	= $(wildcard $(addsuffix /*.cs,$(addprefix $(CODE_DIR)/,$(CS_NAMESPCS)))) $(IDL_CS)

ODL_H		:= $(addsuffix .h,$(ODL_MODULES))
ODL_C		:= $(addsuffix .c,$(ODL_MODULES))
ODL_O		:= $(addsuffix $(OBJ_POSTFIX),$(ODL_MODULES))

ifndef OBJECTS
OBJECTS                := $(C_FILES:%.c=%$(OBJ_POSTFIX)) $(CPP_FILES:%.cpp=%$(OBJ_POSTFIX)) $(Y_FILES:%.y=%$(OBJ_POSTFIX)) $(ODL_O) $(IDL_O) $(EXTRAOBJECTS)
endif

DEPENDENCIES	:= $(C_FILES:%.c=%.d) $(CPP_FILES:%.cpp=%.d) $(Y_FILES:%.y=%.d) $(ODL_C:%.c=%.d)
EXECUTABLES     := $(basename $(OBJECTS))
H_FILES		:= $(L_FILES:%.l=%.h)
QAC_FILES	:= $(C_FILES:%.c=%.c.met) # $(CPP_FILES:%.cpp=%.cpp.met)
GCOV_RESULT	:= $(GCOV_FILES:%.bb=%.c.gcov)

$(H_FILES): $(L_FILES) #$(ODL_FILES) $(IDL_FILES)
#$(OBJECTS): $(C_FILES) $(CPP_FILES) $(Y_FILES:%.y=%.c) $(ODL_C)
#$(DEPENDENCIES): $(C_FILES) $(CPP_FILES) $(Y_FILES:%.y=%.c) $(H_FILES) $(ODL_H)
$(DEPENDENCIES): $(H_FILES) $(ODL_H) $(IDL_H) $(JNI_H)

$(ODL_H): $(ODL_FILES)
	sh $(OSPL_HOME)/bin/sppodl $(SPPODL_FLAGS) $<

$(ODL_C): $(ODL_FILES)
	sh $(OSPL_HOME)/bin/sppodl $(SPPODL_FLAGS) $<

jar: $(JAR_FILE)

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
$(JAR_FILE): $(JAR_DEPENDENCIES) $(CLASS_DIR) $(CLASS_FILES) $(JAR_TARGET) $(MANIFEST)
	$(JAR) cmf $(MANIFEST) $(JAR_FILE) -C bld/$(SPLICE_TARGET) .
else
$(JAR_FILE): $(JAR_DEPENDENCIES) $(CLASS_DIR) $(CLASS_FILES) $(JAR_TARGET) $(MANIFEST)
	$(JAR) cmf $(MANIFEST) $(shell cygpath -m $(JAR_FILE)) -C bld/$(SPLICE_TARGET) .
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
CURRENT_JAVA_DDS_VERSION_GIT = $(shell echo "$(CURRENT_JAVA_DDS_VERSION), non-PrismTech build")
endif


ifdef MANIFEST_CLASSPATH
$(MANIFEST):
	@mkdir -p manifest/$(SPLICE_TARGET)
	@touch -a $(MANIFEST)
	@printf "%s\n%s\n%s\n" "$(CURRENT_JAVA_DDS_VERSION_GIT)" "$(MANIFEST_CLASSPATH)" "$(MANIFEST_MAIN)"  > $(MANIFEST)
else
$(MANIFEST):
	@mkdir -p manifest/$(SPLICE_TARGET)
	@touch -a $(MANIFEST)
	@printf "%s\n%s\n" "$(CURRENT_JAVA_DDS_VERSION_GIT)" "$(MANIFEST_MAIN)"  > $(MANIFEST)

endif

$(JAR_TARGET):
	@mkdir -p $(JAR_LOCATION)/jar/$(SPLICE_TARGET)

