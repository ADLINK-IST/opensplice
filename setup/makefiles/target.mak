include $(OSPL_HOME)/setup/makefiles/rules.mak

ifneq (,$(wildcard /etc/lsb-release))
CPPFLAGS	 += -D_GNU_SOURCE
endif

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
ifneq (,$(findstring win32,$(SPLICE_TARGET))) #windows
	ospl_winmt -manifest $(addsuffix .manifest, $(TARGET)) "-outputresource:$(TARGET);#2"
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
	$(LD_EXE) $(LDFLAGS) $^ $(LDLIBS) $(LDLIBS_SYS) -o $@
endif
ifneq (,$(findstring win32,$(SPLICE_TARGET))) #windows
	ospl_winmt -manifest $(addsuffix .manifest, $(TARGET)) "-outputresource:$(TARGET);#1"
endif 
endif

TARGET_LINK_FILE ?= $(TARGET_LINK_DIR)/$(TARGET)

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

$(TARGET_LINK_DIR):
	mkdir -p $(TARGET_LINK_DIR)

ifdef TARGET_DLIB

ifneq (,$(findstring win32,$(SPLICE_TARGET)))
$(TARGET_LINK_FILE): $(TARGET)
	rm -f $@
	rm -f $(TARGET_LINK_DIR)/$(addsuffix .lib, $(TARGET_DLIB))
	rm -f $(TARGET_LINK_DIR)/$(addsuffix .pdb, $(TARGET_DLIB))
	cp `pwd`/$(TARGET) $@
	if [ -f `pwd`/$(addsuffix .lib, $(TARGET_DLIB)) ]; then cp `pwd`/$(addsuffix .lib, $(TARGET_DLIB)) $(TARGET_LINK_DIR)/$(addsuffix .lib, $(TARGET_DLIB)); fi
	if [ -f `pwd`/$(addsuffix .lib, $(TARGET_DLIB)) ]; then cp `pwd`/$(addsuffix .lib, $(TARGET_DLIB)) $(TARGET_LINK_DIR)/$(addsuffix .lib, $(TARGET_DLIB)); fi
else # NOT windows
$(TARGET_LINK_FILE): $(TARGET)
ifneq (,$(TARGET_LINK_DIR))
	rm -f $@
	ln -s `pwd`/$(TARGET) $@
endif
endif #windows or not

else # TARGET_DLIB

ifneq (, $(findstring win32, $(SPLICE_TARGET)))
$(TARGET_LINK_FILE): $(TARGET)
ifdef TARGET_EXEC
	rm -f $(TARGET_LINK_DIR)/$(addsuffix .pdb, $(TARGET))
	if [ -f `pwd`/$(addsuffix .pdb, $(TARGET)) ]; then cp `pwd`/$(addsuffix .pdb, $(TARGET)) $(addsuffix .pdb, $(TARGET)); fi
endif
	rm -f $@
	cp `pwd`/$(TARGET) $@
else # NOT windows

$(TARGET_LINK_FILE): $(TARGET)
ifneq (,$(TARGET_LINK_DIR))
	rm -f $@
	ln `pwd`/$(TARGET) $@
endif
endif # windows or not

endif # TARGET_DLIB

.PHONY: all compile link clean metre splint qac gcov analyse complexity correctness

all:		link
compile:	$(DEPENDENCIES) $(OBJECTS)
link:		compile $(TARGET) $(TARGET_LINK_DIR) $(TARGET_LINK_FILE)
metre:		$(METRE_FILES)
splint:		$(SPLINT_FILES)
qac:		$(QAC_FILES)
gcov:		$(GCOV_RESULT)
