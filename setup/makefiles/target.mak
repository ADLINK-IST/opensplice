include $(OSPL_HOME)/setup/makefiles/rules.mak

ifdef STATIC_LIB_ONLY
   ifneq "$(TARGET_DLIB)" ""
      TARGET_SLIB:=$(TARGET_DLIB)
      TARGET_DLIB=
   endif
endif

ifdef TARGET_DLIB
TARGET := $(DLIB_PREFIX)$(TARGET_DLIB)$(DLIB_POSTFIX)

ifneq "$(OBJECTS)" ""
TARGET_LINK_DIR ?= $(SPLICE_LIBRARY_PATH)

$(TARGET): $(OBJECTS)
	$(LD_SO) $(LDFLAGS) $^ $(LDLIBS) -o $@
ifneq (,$(findstring win,$(SPLICE_TARGET))) #windows
	ospl_wincmd mt -manifest $(addsuffix .manifest, $(TARGET)) "-outputresource:$(TARGET);#2"
endif
ifneq (,$(findstring vxworks5,$(SPLICE_TARGET)))
ifdef EXTRACTED_LIB
	mkdir -p $(OSPL_HOME)/extract/$(EXTRACTED_LIB)/bld/$(SPLICE_TARGET)
	cp $(OBJECTS) $(OSPL_HOME)/extract/$(EXTRACTED_LIB)/bld/$(SPLICE_TARGET)
endif
endif
endif
endif

ifdef TARGET_SLIB
TARGET := $(SLIB_PREFIX)$(TARGET_SLIB)$(SLIB_POSTFIX)
TARGET_LINK_DIR ?= $(SPLICE_LIBRARY_PATH)

$(TARGET): $(OBJECTS)
	$(AR) $(AR_CMDS) $@ $(OBJECTS)
endif

ifneq (,$(findstring int5,$(SPLICE_TARGET)))
ifdef TARGET_EXEC
BSP_FLAGS  += -non_shared
endif
endif 

ifdef TARGET_EXEC
TARGET := $(EXEC_PREFIX)$(TARGET_EXEC)$(EXEC_POSTFIX)
TARGET_LINK_DIR	?= $(SPLICE_EXEC_PATH)

$(TARGET): $(OBJECTS)
ifneq (,$(findstring int5,$(SPLICE_TARGET)))
	$(LD_EXE) $(LDFLAGS) $(OBJECTS) -o $@
else
	$(LD_EXE) $(LDFLAGS_EXE) $(LDFLAGS) $(OBJECTS) $(LDLIBS) $(LDLIBS_SYS) -o $@
endif
ifneq (,$(findstring win,$(SPLICE_TARGET))) #windows
	ospl_wincmd mt -manifest $(addsuffix .manifest, $(TARGET)) "-outputresource:$(TARGET);#1"
endif 
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
	$(CSC) $(CSFLAGS) -out:$(TARGET) $(CSTARGET_EXEC) $(CS_LIBS) $(CS_FILES)

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

ifneq (,$(TARGET_LINK_DIR))
$(TARGET_LINK_DIR):
	mkdir -p $(TARGET_LINK_DIR)

ifdef TARGET_DLIB
# .pdf and .lib files are copied conditionally, since they are not available for every configuration.
WIN_LIB = $(TARGET_DLIB).lib
WIN_PDB = $(TARGET_DLIB).pdb
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	$(LN) `pwd`/$(TARGET) $@
ifneq (,$(findstring win,$(SPLICE_TARGET)))
	@rm -f $(TARGET_LINK_DIR)/$(WIN_LIB)
	@rm -f $(TARGET_LINK_DIR)/$(WIN_PDB)
	@if [ -f `pwd`/$(WIN_LIB) ]; then $(LN) `pwd`/$(WIN_LIB) $(TARGET_LINK_DIR)/$(WIN_LIB); fi
	@if [ -f `pwd`/$(WIN_PDB) ]; then $(LN) `pwd`/$(WIN_PDB) $(TARGET_LINK_DIR)/$(WIN_PDB); fi
endif
endif # TARGET_DLIB

ifdef TARGET_SLIB
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	$(LN) `pwd`/$(TARGET) $@
endif # TARGET_SLIB

ifdef TARGET_EXEC
# .pdf files are copied conditionally, since they are not available for every configuration.
WIN_PDB = $(TARGET_EXEC).pdb
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	$(LN) `pwd`/$(TARGET) $@
	@rm -f $(TARGET_LINK_DIR)/$(WIN_PDB)
	@if [ -f `pwd`/$(WIN_PDB) ]; then $(LN) `pwd`/$(WIN_PDB) $(TARGET_LINK_DIR)/$(WIN_PDB); fi
endif # TARGET_EXEC

ifdef TARGET_CSLIB
# .pdf and .lib files are copied conditionally, since they are not available for every configuration.
WIN_PDB = $(CSDBG_PREFIX)$(TARGET_CSLIB)$(CSLIB_DBG_POSTFIX)
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	$(LN) `pwd`/$(TARGET) $@
ifneq (, $(findstring win, $(SPLICE_TARGET)))
	@rm -f $(TARGET_LINK_DIR)/$(WIN_PDB)
	@if [ -f `pwd`/$(WIN_PDB) ]; then $(LN) `pwd`/$(WIN_PDB) $(TARGET_LINK_DIR)/$(WIN_PDB); fi
endif 
endif # TARGET_CSLIB

ifdef TARGET_CSEXEC
WIN_PDB = $(CSDBG_PREFIX)$(TARGET_CSEXEC)$(CSEXEC_DBG_POSTFIX)
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	$(LN) `pwd`/$(TARGET) $@
ifneq (, $(findstring win, $(SPLICE_TARGET)))
	@rm -f $(TARGET_LINK_DIR)/$(WIN_PDB)
	@if [ -f `pwd`/$(WIN_PDB) ]; then $(LN) `pwd`/$(WIN_PDB) $(TARGET_LINK_DIR)/$(WIN_PDB); fi
endif 
endif # TARGET_CSEXEC

ifdef TARGET_CSMOD
WIN_PDB = $(CSDBG_PREFIX)$(TARGET_CSMOD)$(CSMOD_DBG_POSTFIX)
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	$(LN) `pwd`/$(TARGET) $@
ifneq (, $(findstring win, $(SPLICE_TARGET)))
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
