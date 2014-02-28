ifdef STATIC_LIB_ONLY
   ifneq "$(TARGET_DLIB)" ""
      TARGET_SLIB:=$(TARGET_DLIB)
      TARGET_DLIB=
   endif
else
ifdef DYNAMIC_LIB_ONLY
   ifneq "$(TARGET_SLIB)" ""
      TARGET_DLIB:=$(TARGET_SLIB)
      TARGET_SLIB:=
      LDFLAGS += $(SHLDFLAGS)
   endif
endif
endif

FILTERED_LDLIBS = $(filter-out $(REQUIRED_SHARED_LIBS_BASE:%=-l%), $(LDLIBS))

### MAKE_DLIB ###
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
  define make_dlib
	$(LD_SO) $(LDFLAGS) $^ $(FILTERED_LDLIBS) -o $@
	ospl_wincmd mt -manifest $(addsuffix .manifest, $@) "-outputresource:$@;#2"
  endef
else
  define make_dlib
	$(LD_SO) $(LDFLAGS) $^ $(FILTERED_LDLIBS) -o $@
  endef
endif
### MAKE_DLIB ###

ifdef TARGET_DLIB

TARGET := $(DLIB_PREFIX)$(TARGET_DLIB)$(DLIB_POSTFIX)

ifneq "$(OBJECTS)" ""
TARGET_LINK_DIR ?= $(SPLICE_LIBRARY_PATH)

$(TARGET): $(OBJECTS)
	$(make_dlib)
endif
endif

define make_slib
	$(AR) $(AR_CMDS) $@ $^
endef # make_slib

ifdef TARGET_SLIB
TARGET := $(SLIB_PREFIX)$(TARGET_SLIB)$(SLIB_POSTFIX)
TARGET_LINK_DIR ?= $(SPLICE_LIBRARY_PATH)

$(TARGET): $(OBJECTS)
	$(make_slib)
endif

ifneq (,$(findstring int5,$(SPLICE_TARGET)))
ifdef TARGET_EXEC
BSP_FLAGS  += -non_shared
endif
endif 

### MAKE_EXEC ###
ifneq (,$(findstring rtems,$(SPLICE_TARGET)))
  define make_exec
	$(LD_EXE) $(LDFLAGS) $@ $^
  endef
else
  ifneq (,$(findstring int5,$(SPLICE_TARGET)))
    define make_exec
	$(LD_EXE) $(LDFLAGS) $^ -o $@
    endef
  else
    ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
      define make_exec
	$(LD_EXE) $(LDFLAGS_EXE) $(LDFLAGS) $^ $(FILTERED_LDLIBS) $(LDLIBS_SYS) -o $@
	ospl_wincmd mt -manifest $(addsuffix .manifest, $@) "-outputresource:$@;#1"
      endef
    else
      define make_exec
	$(LD_EXE) $(LDFLAGS_EXE) $(LDFLAGS) $^ $(FILTERED_LDLIBS) $(LDLIBS_SYS) -o $@
      endef
    endif
  endif
endif
### MAKE_EXEC ###

ifdef TARGET_EXEC

TARGET := $(EXEC_PREFIX)$(TARGET_EXEC)$(EXEC_POSTFIX)
TARGET_LINK_DIR	?= $(SPLICE_EXEC_PATH)

$(TARGET): $(OBJECTS)
	$(make_exec)
endif

# Define the libpath CS_LIBS variable that that creates private links to the referenced libs. 
CS_MAKE_SEPARATOR = ,
CS_LIB_SRCS = $(subst $(CS_MAKE_SEPARATOR), ,$(subst -r:,,$(CSLIBS)))
CS_LIB_FILES = $(notdir $(CS_LIB_SRCS))
CS_LIB_DIRS = $(dir $(CS_LIB_SRCS))
CS_LIBS = $(addprefix -r:,$(notdir $(CS_LIB_FILES)))

vpath $(CSLIB_PREFIX)%$(CSLIB_POSTFIX) $(CS_LIB_DIRS)

ifdef TARGET_CSLIB
TARGET := $(CSLIB_PREFIX)$(TARGET_CSLIB)$(CSLIB_POSTFIX)
TARGET_LINK_DIR ?= $(SPLICE_LIBRARY_PATH)

ifneq "$(CS_FILES)" ""

# Create local links to referenced libraries.
# Remove old links first to make sure you always point to the most recent libraries.
$(TARGET): $(CS_FILES)
	@for FILE in $(CS_LIB_SRCS); do LFILE=`basename $${FILE}`; rm -f $${LFILE}; $(LN) $${FILE} $${LFILE}; done
	$(CSC) $(CSFLAGS) -out:$(TARGET) $(CSTARGET_LIB) $(CS_LIBS) $(CS_FILES)
endif
endif

ifdef TARGET_CSMOD
TARGET := $(CSMOD_PREFIX)$(TARGET_CSMOD)$(CSMOD_POSTFIX)
TARGET_LINK_DIR ?= $(SPLICE_LIBRARY_PATH)

ifneq "$(CS_FILES)" ""

$(TARGET): $(CS_FILES)
	$(CSC) $(CSFLAGS) -out:$(TARGET) $(CSTARGET_MOD) $(CS_LIBS) $(CS_FILES)
endif
endif

ifdef TARGET_CSEXEC
TARGET := $(CSEXEC_PREFIX)$(TARGET_CSEXEC)$(CSEXEC_POSTFIX)
TARGET_LINK_DIR	?= $(SPLICE_EXEC_PATH)

ifneq "$(CS_FILES)" ""

# Create local links to referenced libraries. 
# Remove old links first to make sure you always point to the most recent libraries.
$(TARGET): $(CS_FILES)
	@for FILE in $(CS_LIB_SRCS); do LFILE=`basename $${FILE}`; rm -f $${LFILE}; $(LN) $${FILE} $${LFILE}; done
	$(CSC) $(CSFLAGS) -out:$(TARGET) $(CSTARGET_EXEC) -r:System.dll $(CS_LIBS) $(CS_FILES)

# Create additional local links to the referenced libraries in the directory of the executables. 
CS_LIB_LINKS = $(addprefix $(TARGET_LINK_DIR)/, $(notdir $(CS_LIB_SRCS)))

$(TARGET_LINK_DIR)/$(CSLIB_PREFIX)%$(CSLIB_POSTFIX): $(CSLIB_PREFIX)%$(CSLIB_POSTFIX)
	rm -f $@
	$(LN) $< $@
endif
endif

ifneq (,$(TARGET_LINK_DIR))
TARGET_LINK_FILE ?= $(TARGET_LINK_DIR)/$(TARGET)
endif

ifneq ($(EXEC_POSTFIX),'.a')
ifdef STATIC_LIB_ONLY
ifdef TARGET_EXEC
TARGET_LIB_DEPS=$(addsuffix .a,$(subst -l,lib,$(LDLIBS)))
TARGET_DEP=$(addsuffix .d,$(TARGET_LINK_FILE))
DEPENDENCIES+=$(TARGET_DEP)
$(TARGET): | $(TARGET_DEP)

$(TARGET_DEP): $(OSPL_HOME)/setup/makefiles/target.mak $(MAKEFILE_LIST)
	@echo "$(TARGET): \\" > $(TARGET_DEP); \
	TLD="$(TARGET_LIB_DEPS)"; \
	for lib in $$TLD; \
	do \
	   found=0; \
	   for lpath in `echo "$(LDFLAGS)" | tr -s ' ' '\n' | grep -- -L | sed 's/^-L//'`; \
	   do \
	      if [ "$$found" != "1" -a -f "$$lpath/$$lib" ]; \
	      then \
                 echo "$$lpath/$$lib \\" | sed 's@^$(OSPL_HOME)@$$(OSPL_HOME)@' >> $(TARGET_DEP); \
	         found=1; \
              fi \
           done \
	done
endif
endif
endif

### abspath_make3p80_wrapper: if $(abspath) doesn't work (for make
### 3.80, which sadly we still support), use $(shell pwd).  The two
### aren't the same, but we use it only for linking/copying files to
### the exec/lib directories, and only ever with an argument that is a
### plain file name without a directory part.
ifeq "$(abspath /)" ""
  define abspath_make3p80_wrapper
	$(shell pwd)/$1
  endef
else
  define abspath_make3p80_wrapper
	$(abspath $1)
  endef
endif

### MAKE_DLIB_LINK ###
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
  define make_dlib_link
	rm -f $@
	$(LN) $(call abspath_make3p80_wrapper,$<) $@
	rm -f $(@:%$(DLIB_POSTFIX)=%.pdb)
	if [ -f $(<:%$(DLIB_POSTFIX)=%.pdb) ] ; then \
			$(LN) $(call abspath_make3p80_wrapper,$(<:%$(DLIB_POSTFIX)=%.pdb)) $(@:%$(DLIB_POSTFIX)=%.pdb) ; \
		fi
	rm -f $(@:%$(DLIB_POSTFIX)=%.lib)
	if [ -f $(<:%$(DLIB_POSTFIX)=%.lib) ] ; then \
			$(LN) $(call abspath_make3p80_wrapper,$(<:%$(DLIB_POSTFIX)=%.lib)) $(@:%$(DLIB_POSTFIX)=%.lib) ; \
		fi
  endef
else
  define make_dlib_link
	rm -f $@
	$(LN) $(call abspath_make3p80_wrapper,$<) $@
  endef
endif
### MAKE_DLIB_LINK ###

### MAKE_SLIB_LINK ###
define make_slib_link
	rm -f $@
	$(LN) $(call abspath_make3p80_wrapper,$<) $@
endef
### MAKE_SLIB_LINK ###

### MAKE_EXEC_LINK ###
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
  define make_exec_link
	rm -f $@
	$(LN) $(call abspath_make3p80_wrapper,$<) $@
	rm -f $(@:%$(EXEC_POSTFIX)=%.pdb)
	if [ -f $(<:%$(EXEC_POSTFIX)=%.pdb) ] ; then \
			$(LN) $(call abspath_make3p80_wrapper,$(<:%$(EXEC_POSTFIX)=%.pdb)) $(@:%$(EXEC_POSTFIX)=%.pdb) ; \
		fi
  endef
else
  define make_exec_link
	rm -f $@
	$(LN) $(call abspath_make3p80_wrapper,$<) $@
  endef
endif
### MAKE_EXEC_LINK ###

ifneq (,$(TARGET_LINK_DIR))
$(TARGET_LINK_DIR):
	mkdir -p $(TARGET_LINK_DIR)

ifdef TARGET_DLIB
$(TARGET_LINK_FILE): $(TARGET)
	$(make_dlib_link)
endif # TARGET_DLIB

ifdef TARGET_SLIB
$(TARGET_LINK_FILE): $(TARGET)
	$(make_slib_link)
endif # TARGET_SLIB

ifdef TARGET_EXEC
$(TARGET_LINK_FILE): $(TARGET)
	$(make_exec_link)
endif # TARGET_EXEC

ifdef TARGET_EXEC_NOLN
# .pdf files are copied conditionally, since they are not available for every configuration.
WIN_PDB = $(TARGET_EXEC).pdb
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	$(CP) `pwd`/$(TARGET) $@
	@rm -f $(TARGET_LINK_DIR)/$(WIN_PDB)
	@if [ -f `pwd`/$(WIN_PDB) ]; then $(CP) `pwd`/$(WIN_PDB) $(TARGET_LINK_DIR)/$(WIN_PDB); fi
endif # TARGET_EXEC_NOLN

ifdef TARGET_CSLIB
# .pdf and .lib files are copied conditionally, since they are not available for every configuration.
WIN_PDB = $(CSDBG_PREFIX)$(TARGET_CSLIB)$(CSLIB_DBG_POSTFIX)
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	$(LN) `pwd`/$(TARGET) $@
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
	@rm -f $(TARGET_LINK_DIR)/$(WIN_PDB)
	@if [ -f `pwd`/$(WIN_PDB) ]; then $(LN) `pwd`/$(WIN_PDB) $(TARGET_LINK_DIR)/$(WIN_PDB); fi
endif 
endif # TARGET_CSLIB

ifdef TARGET_CSEXEC
WIN_PDB = $(CSDBG_PREFIX)$(TARGET_CSEXEC)$(CSEXEC_DBG_POSTFIX)
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	$(LN) `pwd`/$(TARGET) $@
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
	@rm -f $(TARGET_LINK_DIR)/$(WIN_PDB)
	@if [ -f `pwd`/$(WIN_PDB) ]; then $(LN) `pwd`/$(WIN_PDB) $(TARGET_LINK_DIR)/$(WIN_PDB); fi
endif 
endif # TARGET_CSEXEC

ifdef TARGET_CSMOD
WIN_PDB = $(CSDBG_PREFIX)$(TARGET_CSMOD)$(CSMOD_DBG_POSTFIX)
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	$(LN) `pwd`/$(TARGET) $@
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
	@rm -f $(TARGET_LINK_DIR)/$(WIN_PDB)
	@if [ -f `pwd`/$(WIN_PDB) ]; then $(LN) `pwd`/$(WIN_PDB) $(TARGET_LINK_DIR)/$(WIN_PDB); fi
endif 
endif # TARGET_CSMOD

endif # TARGET_LINK_DIR
PROC_CORE?=$(PROC)

.PHONY: all compile link clean metre splint qac gcov analyse complexity correctness

all:		link
compile:	$(DEPENDENCIES) $(OBJECTS)
link:		compile $(TARGET) $(TARGET_LINK_DIR) $(TARGET_LINK_FILE) $(CS_LIB_LINKS)
metre:		$(METRE_FILES)
splint:		$(SPLINT_FILES)
qac:		$(QAC_FILES)
gcov:		$(GCOV_RESULT)
