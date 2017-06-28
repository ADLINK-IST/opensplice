.NOTPARALLEL:

JAR_BASE       := dcpscj5
CLASSIC_BASE   := dcpscj
CLASSIC_HOME_DIR:= $(OSPL_HOME)/src/api/dcps/java/cj
JPACKAGES      := org/omg/dds/core \
                  org/omg/dds/core/event \
                  org/omg/dds/core/policy \
                  org/omg/dds/core/status \
                  org/omg/dds/type \
                  org/omg/dds/type/builtin \
                  org/omg/dds/type/dynamic \
                  org/omg/dds/type/typeobject \
                  org/omg/dds/topic \
                  org/omg/dds/domain \
                  org/omg/dds/pub \
                  org/omg/dds/sub \
                  org/opensplice/dds/core \
                  org/opensplice/dds/core/event \
                  org/opensplice/dds/core/policy \
                  org/opensplice/dds/core/status \
                  org/opensplice/dds/domain \
                  org/opensplice/dds/pub \
                  org/opensplice/dds/sub \
                  org/opensplice/dds/topic \
                  org/opensplice/dds/type

JAVA_ORB = JACORB

JCODE_COMMON_DIR = ../common/java/code
JCODE_PATH      = $(JCODE_COMMON_DIR):$(JCODE_DIR)
JCODE_PATH_NORMALIZED = $(call JNORMALIZE,$(JCODE_COMMON_DIR))$(JSEPARATOR)$(call JNORMALIZE,$(JCODE_DIR))
CLASS_PATH_NORMALIZED = $(call JNORMALIZE,$(CLASSIC_JAR_FILE))$(JSEPARATOR)$(call JNORMALIZE,$(JACORB_HOME)/lib/endorsed/jacorb.jar)

JAVA_DIRS       = $(shell find $(JCODE_DIR) -type d) 
JAVA_DIRS      += $(shell find $(JCODE_COMMON_DIR) -type d)
JAVA_FILES      = $(wildcard $(addsuffix /*.java,$(addprefix $(JCODE_DIR)/,$(JPACKAGES))))
JAVA_FILES     += $(wildcard $(addsuffix /*.java,$(addprefix $(JCODE_COMMON_DIR)/,$(JPACKAGES))))

all link: jar src

include ../common.mak
