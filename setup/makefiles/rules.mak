ifneq (,$(wildcard $(OSPL_HOME)/setup/$(SPLICE_TARGET)))
include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak
else
include $(OSPL_OUTER_HOME)/setup/$(SPLICE_TARGET)/config.mak
endif

ifdef FLEX_HOME
FLEX_LM_NEW     ?= lm_new$(OBJ_POSTFIX)
endif

# This makefile defined the platform and component independent make rules.
CLASS_DIR  =bld/$(SPLICE_TARGET)
JCODE_DIR ?=code
JCODE_PATH ?= $(JCODE_DIR)
JFLAGS    ?= -source 1.5 -target 1.5
JFLAGS    +=-sourcepath '$(JCODE_PATH)'
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
ifeq (,$(findstring win32,$(SPLICE_HOST))) 
MANIFEST_CLASSPATH=Class-Path: $(notdir $(subst :, ,$(JAVA_INC)))
#MANIFEST_CLASSPATH=Class-Path: $(subst $(JAR_INC_DIR)/,,$(subst :, ,$(JAVA_INC)))
else # it is windows
MANIFEST_CLASSPATH=Class-Path: $(notdir $(foreach entry,$(subst ;, ,$(JAVA_INC)),$(shell cygpath "$(entry)")))
#MANIFEST_CLASSPATH=Class-Path: $(notdir $(subst ;, ,$(JAVA_INC)))
endif
endif

$(CLASS_DIR):
	mkdir -p $(CLASS_DIR)

LOCAL_CLASS_DIR	=$(CLASS_DIR)/$(PACKAGE_DIR)

.PRECIOUS: %.c %.h

(%.o): %.o
	$(AR) r $@ $<

ifeq (,$(findstring win32,$(SPLICE_HOST)))
ifeq (,$(findstring studio,$(SPLICE_HOST)))
%.d: %.c
	$(CPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CINCS) $< >$@
else
%.d: %.c
	$(CPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CINCS) $< | sed -e 's/\([^:\\]\) /\1\\ /g' >$@
endif
else
%.d: %.c
	$(CPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CINCS) $< | grep "^#line.*\\\\ospl[io]\\\\" | cut -d '"' -f 2 | sort -u | sed -e 's@\([A-Za-z]\)\:@ /cygdrive/\1@' -e 's@\\\\@/@g' -e '$$!s@$$@ \\@' -e '1s@^@$*$(OBJ_POSTFIX): @' >$@
endif

ifeq (,$(findstring win32,$(SPLICE_HOST)))
ifeq (,$(findstring studio,$(SPLICE_HOST)))
%.d: %.cpp
	$(GCPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CXXINCS) $< >$@
else
%.d: %.cpp
	$(GCPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CXXINCS) $< | sed -e 's/\([^:\\]\) /\1\\ /g' >$@
endif
else
%.d: %.cpp
	$(GCPP) $(MAKEDEPFLAGS) $(CPPFLAGS) $(CXXINCS) $< | grep "^#line.*\\\\ospl[io]\\\\" | cut -d '"' -f 2 | sort -u | sed -e 's@\([A-Za-z]\)\:@ /cygdrive/\1@' -e 's@\\\\@/@g' -e '$$!s@$$@ \\@' -e '1s@^@$*$(OBJ_POSTFIX): @' >$@
endif

%$(OBJ_POSTFIX): %.c
	$(FILTER) $(CC) $(CPPFLAGS) $(CFLAGS) $(CINCS) -c $<

%$(OBJ_POSTFIX): %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -c $<

%$(OBJ_POSTFIX): %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -c $<

%.c: %.y
	$(YACC) $< -o $@

%.h: %.l
	$(LEX) -t  -P$(basename $@)_yy $< > $@

%.c.met: %.c
	$(QAC) $(CPFLAGS) $(CINCS) $<

%.c.gcov: %.bb
	$(GCOV) -b -f $< > $@.sum

$(CLASS_DIR)/%.class: $(JCODE_DIR)/%.java 
ifeq (,$(findstring win32,$(SPLICE_HOST))) 
	$(JCC) -classpath "$(CLASS_DIR):$(JAVA_INC):$(JAVA_SYSTEM_CP)" $(JFLAGS) -d $(CLASS_DIR) $(JCFLAGS) $(dir $<)*.java
else
	$(JCC) -classpath '$(CLASS_DIR);$(JAVA_INC);$(JAVA_SYSTEM_CP)' $(JFLAGS) -d $(CLASS_DIR) $(JCFLAGS) $(dir $<)*.java
endif

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
CINCS		+= -I$(OSPL_HOME)/src/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os-net/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os-net/$(OS)$(OS_REV)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/include
CINCS		+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
CINCS		+= -I$(OSPL_HOME)/src/abstraction/pa/$(PROC_CORE)
else
CINCS		+= -I$(OSPL_HOME)/src/include
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
endif

CXXINCS	 = -I.
CXXINCS	+= -I../../include
CXXINCS	+= -I$(CODE_DIR)

ifndef OSPL_OUTER_HOME
CXXINCS	+= -I$(OSPL_HOME)/src/include
CXXINCS  += -I$(OSPL_HOME)/src/abstraction/os/include
CXXINCS	+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
else
CXXINCS	+= -I$(OSPL_HOME)/src/include
CXXINCS  += -I$(OSPL_OUTER_HOME)/src/abstraction/os/include
CXXINCS  += -I$(OSPL_HOME)/src/abstraction/os/include
CXXINCS	+= -I$(OSPL_OUTER_HOME)/src/abstraction/os/$(OS)$(OS_REV)
CXXINCS	+= -I$(OSPL_HOME)/src/abstraction/os/$(OS)$(OS_REV)
endif

C_FILES		?= $(notdir $(wildcard $(CODE_DIR)/*.c)) 
CPP_FILES	?= $(notdir $(wildcard $(CODE_DIR)/*.cpp))
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
OBJECTS                := $(C_FILES:%.c=%$(OBJ_POSTFIX)) $(CPP_FILES:%.cpp=%$(OBJ_POSTFIX)) $(Y_FILES:%.y=%$(OBJ_POSTFIX)) $(ODL_O) $(IDL_O)
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
	cp $(ODL_H)  $(OSPL_HOME)/src/include

$(ODL_C): $(ODL_FILES)
	sh $(OSPL_HOME)/bin/sppodl $(SPPODL_FLAGS) $<
	cp $(ODL_H)  $(OSPL_HOME)/src/include

jar: $(JAR_FILE)

ifeq (,$(findstring win32,$(SPLICE_HOST))) 
$(JAR_FILE): $(JAR_DEPENDENCIES) $(CLASS_DIR) $(CLASS_FILES) $(JAR_TARGET) $(MANIFEST)
	$(JAR) cmf $(MANIFEST) $(JAR_FILE) -C bld/$(SPLICE_TARGET) .
else
$(JAR_FILE): $(JAR_DEPENDENCIES) $(CLASS_DIR) $(CLASS_FILES) $(JAR_TARGET) $(MANIFEST)
	$(JAR) cmf $(MANIFEST) $(shell cygpath -m $(JAR_FILE)) -C bld/$(SPLICE_TARGET) .
endif

ifdef MANIFEST_CLASSPATH
$(MANIFEST):
	@mkdir -p manifest/$(SPLICE_TARGET)
	@touch -a $(MANIFEST)
	@printf "%s\n%s\n" "$(MANIFEST_CLASSPATH)" "$(MANIFEST_MAIN)" > $(MANIFEST)
else
$(MANIFEST):
	@mkdir -p manifest/$(SPLICE_TARGET)
	@touch -a $(MANIFEST)
	@printf "%s\n" "$(MANIFEST_MAIN)" > $(MANIFEST)
endif

$(JAR_TARGET):
	@mkdir -p $(JAR_LOCATION)/jar/$(SPLICE_TARGET)

