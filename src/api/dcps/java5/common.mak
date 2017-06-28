JAVADOC        := javadoc #needs to be moved to the setup files

JAR_MODULE     ?= $(JAR_BASE).jar
SRC_JAR_MODULE ?= $(JAR_BASE)-src.jar
DOC_JAR_MODULE ?= $(JAR_BASE)-doc.jar
JAR_LOCATION   := $(OSPL_HOME)

SRC_JAR_MODULE := $(JAR_BASE)-src.jar
SRC_JAR_FILE    = $(JAR_TARGET)/$(SRC_JAR_MODULE)

DOC_JAR_MODULE := $(JAR_BASE)-doc.jar
DOC_DIR        := $(OSPL_HOME)/docs/java5

JBASE_DIR      := bld/$(SPLICE_TARGET)
CLASS_DIR      := $(JBASE_DIR)/class
DEP_DIR        := $(JBASE_DIR)/dep
JAVA_INC        = $(CLASS_DIR)

JAVA_DIRS      ?= $(shell find $(JCODE_DIR) -type d) 

JAR_DEPENDENCIES += $(CLASSIC_PACKAGES) $(JAVA_DIRECTORIES)

CLASSIC_MODULE := $(CLASSIC_BASE).jar
CLASSIC_SRC_MODULE := $(CLASSIC_BASE)-src.jar
CLASSIC_PACKAGES = $(CLASS_DIR)/DDS
CLASSIC_SRC_DIR := $(JBASE_DIR)/src
CLASSIC_DEP_DIR := $(CLASSIC_HOME_DIR)/$(DEP_DIR)
CLASSIC_JAR_FILE = $(JAR_TARGET)/$(CLASSIC_MODULE)
CLASSIC_SRC_JAR_FILE = $(JAR_TARGET)/$(CLASSIC_SRC_MODULE)

CLASSIC_DEPENDENCIES := $(CLASSIC_DEP_DIR)/$(CLASSIC_MODULE:%.jar=%.dep) $(CLASSIC_DEP_DIR)/$(CLASSIC_MODULE:%.jar=%.ddep) $(CLASSIC_DEP_DIR)/$(CLASSIC_SRC_MODULE:%.jar=%.dep) $(CLASSIC_DEP_DIR)/$(CLASSIC_SRC_MODULE:%.jar=%.ddep)

include $(OSPL_HOME)/setup/makefiles/target.mak

$(DOC_DIR):
	$(AT_SIGN)mkdir -p $@

$(JBASE_DIR):
	$(AT_SIGN)mkdir -p $@

$(CLASSIC_DEP_DIR)/%.dep $(CLASSIC_DEP_DIR)/%.ddep:
	@$(MAKE) --no-print-directory -C $(CLASSIC_HOME_DIR) $@

$(CLASSIC_JAR_FILE) $(CLASSIC_SRC_JAR_FILE):
	@$(MAKE) --no-print-directory -C $(CLASSIC_HOME_DIR) $@

$(CLASSIC_PACKAGES): $(CLASSIC_JAR_FILE) | $(CLASS_DIR)
	$(AT_SIGN)rm -rf $@/*
	$(AT_SIGN)cd $(CLASS_DIR) ; $(JAR) xf $(call JNORMALIZE,$<)
	$(AT_SIGN)rm -rf $(CLASS_DIR)/META_INF
	$(AT_SIGN)touch $@

$(CLASSIC_SRC_DIR): $(CLASSIC_SRC_JAR_FILE)
	$(AT_SIGN)rm -rf $@
	$(AT_SIGN)mkdir -p $@
	$(AT_SIGN)cd $@ ; $(JAR) xf $(call JNORMALIZE,$<)
	$(AT_SIGN)touch $@

ifneq ($(MAKECMDGOALS),clean)
-include $(CLASSIC_DEPENDENCIES)
endif

.PHONY: src
src: $(JAR_TARGET)/$(SRC_JAR_MODULE)

.DELETE_ON_ERROR: $(JAR_TARGET)/%-src.jar
$(JAR_TARGET)/%-src.jar: $(CLASSIC_SRC_JAR_FILE) $(JAVA_FILES) $(JAVA_DIRECTORIES) $(CLASSIC_SRC_DIR) | $(JAR_TARGET)
	@$(ECHO_COMMAND) JAR $@
	$(AT_SIGN)$(JAR) cf $(call JNORMALIZE,$(@)) -C $(call JNORMALIZE,$(JCODE_DIR)) .
	$(AT_SIGN)$(JAR) uf $(call JNORMALIZE,$(@)) -C $(call JNORMALIZE,$(JCODE_COMMON_DIR)) .
	$(AT_SIGN)$(JAR) uf $(call JNORMALIZE,$(@)) -C $(call JNORMALIZE,$(CLASSIC_SRC_DIR)) .

.PHONY: doc
doc: $(DOC_DIR)/$(DOC_JAR_MODULE)

$(JBASE_DIR)/java_files.txt: $(JAVA_FILES) | $(JBASE_DIR)
	@echo $(sort $(call JNORMALIZE,$(filter %.java, $^))) > $@

$(DOC_DIR)/$(DOC_JAR_MODULE): $(CLASSIC_JAR_FILE) $(JBASE_DIR)/java_files.txt $(JAVA_DIRECTORIES) | $(DOC_DIR) $(CLASS_DIR)
	$(AT_SIGN)mkdir -p $(JBASE_DIR)/doc
	$(JAVADOC) -sourcepath "$(JCODE_PATH_NORMALIZED)" -classpath "$(CLASS_PATH_NORMALIZED)" -quiet -d $(JBASE_DIR)/doc @$(JBASE_DIR)/java_files.txt
	$(JAR) cf $(call JNORMALIZE,$@) -C $(call JNORMALIZE,$(JBASE_DIR)/doc) .
	$(AT_SIGN)rm -rf $(JBASE_DIR)/doc

.PHONY: clean
clean:
	@rm -rf $(CLASS_DIR) $(CLASSIC_SRC_DIR) $(JBASE_DIR) $(CLASSIC_PACKAGES) $(MANIFEST_TARGET)
	@rm -vf $(JAR_FILE)
	@rm -vf $(JAR_TARGET)/$(SRC_JAR_MODULE)
	@rm -vf $(DOC_DIR)/$(DOC_JAR_MODULE)