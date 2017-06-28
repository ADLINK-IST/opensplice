# make 3.80 does not
# - allow us to define a new-line
# - support abspath
ifeq (3.80,$(MAKE_VERSION))
DEP_LINEEND :=#
ABSPATH = $(shell cd $(dir $(1)) && pwd)/$(notdir $(1))
else
# define new line character
define nl


endef
DEP_LINEEND :=  \\$(nl)
ABSPATH = $(abspath $(1))
endif


JAR_MODULE     ?= $(JAR_BASE).jar
SRC_JAR_MODULE ?= $(JAR_BASE)-src.jar
JAR_LOCATION    = $(OSPL_HOME)

JBASE_DIR      := bld/$(SPLICE_TARGET)
CLASS_DIR      := $(JBASE_DIR)/class

DEP_DIR        := $(JBASE_DIR)/dep
JAR_DEP        := $(JAR_MODULE:%.jar=%.dep)
JAR_DDEP       := $(JAR_MODULE:%.jar=%.ddep)

SRC_JAR_DEP    := $(SRC_JAR_MODULE:%.jar=%.dep)
SRC_JAR_DDEP   := $(SRC_JAR_MODULE:%.jar=%.ddep)

IDL_DIR        ?= $(OSPL_HOME)/etc/idl
IDL_CODE_DIR   ?= $(JBASE_DIR)/idl
IDL_TIMESTAMP   = $(IDL_CODE_DIR)/.java_idl.timestamp
IDLPP           = idlpp
vpath %.idl $(IDL_DIR)

JAVA_DIRS      ?= $(shell find $(JCODE_DIR) -type d)

# JAVA_FILES has to be absolute for dependency checking by JAVA5
JAVA_FILES_TMP ?= $(wildcard $(addsuffix /*.java,$(addprefix $(JCODE_DIR)/,$(JPACKAGES))))
JAVA_FILES      = $(foreach f,$(JAVA_FILES_TMP),$(call ABSPATH,$(f)))

JAVA_INC       ?= $(CLASS_DIR)

MANIFEST_TEMPLATE = meta-inf/manifest-osgi-bundle.tmpl

JDEPENDENCIES  := $(DEP_DIR)/$(JAR_DDEP) $(DEP_DIR)/$(JAR_DEP) $(DEP_DIR)/$(SRC_JAR_DDEP) $(DEP_DIR)/$(SRC_JAR_DEP)
ifeq ($(findstring clean,$(MAKECMDGOALS)),clean)
JAR_DEPENDENCIES += $(JDEPENDENCIES)
SRC_JAR_DEPENDENCIES += $(JDEPENDENCIES)
endif


include $(OSPL_HOME)/setup/makefiles/target.mak

$(IDL_CODE_DIR):
	@mkdir -p $@

$(IDL_TIMESTAMP): $(IDL_JAVA_FILES)
	$(JCC) $(JCC_ARGS) @$<
	@touch $@

$(DEP_DIR):
	@mkdir -p $@

.DELETE_ON_ERROR: $(DEP_DIR)/%.ddep
$(DEP_DIR)/%.ddep: | $(DEP_DIR)
	@$(ECHO_COMMAND) create $@
	@echo "$(@:%.ddep=%.dep): \\" > $@
	@echo "$(foreach d,$(JAVA_DIRS),$(call ABSPATH,$(d))$(DEP_LINEEND))" >> $@
	@echo "" >> $@

.DELETE_ON_ERROR: $(DEP_DIR)/%.dep
$(DEP_DIR)/%.dep: | $(DEP_DIR)
	@$(ECHO_COMMAND) create $@
	@echo "$(@:$(DEP_DIR)%.dep=$(JAR_TARGET)%.jar): \\" > $@
	@echo -n " $(foreach f,$(IDL_FILES), $(call ABSPATH,$(IDL_DIR)/$(f))$(DEP_LINEEND))" >> $@
	@echo " $(foreach f,$(JAVA_FILES), $(f)$(DEP_LINEEND))" >> $@
	@echo "" >> $@

.PHONY: src
src: $(JAR_TARGET)/$(SRC_JAR_MODULE)

.DELETE_ON_ERROR: $(SRC_JAR_FILE)
$(JAR_TARGET)/%-src.jar: $(SRC_JAR_DEPENDENCIES) | $(JAR_TARGET)
	@$(ECHO_COMMAND) JAR $@
	$(AT_SIGN)$(JAR) cf $(call JNORMALIZE,$(@)) -C $(call JNORMALIZE,$(JCODE_DIR)) .
	$(AT_SIGN)$(JAR) uf $(call JNORMALIZE,$(@)) -C $(call JNORMALIZE,$(JCODE_COMMON_DIR)) .
	$(AT_SIGN)$(JAR) uf $(call JNORMALIZE,$(@)) -C $(call JNORMALIZE,$(IDL_CODE_DIR)) .

.PHONY: clean
clean:
	@rm -rf $(CLASS_DIR) $(DEP_DIR) $(IDL_CODE_BASE_DIR) $(JBASE_DIR) $(MANIFEST_TARGET)
	@rm -vf $(JAR_TARGET)/$(JAR_MODULE)
	@rm -vf $(JAR_TARGET)/$(SRC_JAR_MODULE)

ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
-include $(JDEPENDENCIES)
endif
