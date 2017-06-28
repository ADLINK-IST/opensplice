JAR_BASE       := dcpscj_y2038_ready

JCODE_DIR      := ../cj/code
JCODE_COMMON_DIR =  ../common/java/code
JPACKAGES       = DDS org/opensplice/dds/dcps org/opensplice/dds/type

JAVA_DIRS       = $(shell find $(JCODE_DIR) -type d)
JAVA_DIRS      += $(shell find $(JCODE_COMMON_DIR) -type d)
JAVA_FILES_TMP  = $(wildcard $(addsuffix /*.java,$(addprefix $(JCODE_DIR)/,$(JPACKAGES))))
JAVA_FILES_TMP += $(wildcard $(addsuffix /*.java,$(addprefix $(JCODE_COMMON_DIR)/,$(JPACKAGES))))

IDL_FILES      := dds_builtinTopics.idl dds_namedQosTypes.idl
IDL_JAVA_FILES  = $(IDL_CODE_DIR)/idl_java_files.txt
GEN_IDL_FILES  := $(IDL_FILES:%.idl=%Dcps.idl)
GEN_IDL_FILES_PREFIXED = $(addprefix $(IDL_CODE_DIR)/,$(GEN_IDL_FILES))

JCODE_PATH      = $(JCODE_DIR):$(JCODE_COMMON_DIR):$(IDL_CODE_DIR)

JAR_DEPENDENCIES += $(IDL_TIMESTAMP)
SRC_JAR_DEPENDENCIES += $(IDL_TIMESTAMP)

# JACORB
JAVA_ORB = JACORB

all link: jar src

include ../common.mak

$(IDL_CODE_DIR)/%Dcps.idl: %.idl | $(IDL_CODE_DIR)
	$(IDLPP) -l java -C -d $(IDL_CODE_DIR) `ospl_normalizePath $<`

$(IDL_JAVA_FILES): $(GEN_IDL_FILES_PREFIXED) | $(CLASS_DIR)
	@for f in $^ ; do \
		echo $(JAVA) $(CP_JACORB_IDL) org.jacorb.idl.parser -DOSPL_DDS_Y2038_READY -I"$(OSPL_HOME_NORMALIZED)/etc/idl" -all -d $(IDL_CODE_DIR) $$f ; \
		$(JAVA) $(CP_JACORB_IDL) org.jacorb.idl.parser -DOSPL_DDS_Y2038_READY -I"$(OSPL_HOME_NORMALIZED)/etc/idl" -all -d $(IDL_CODE_DIR) $$f ; \
	done
	find $(IDL_CODE_DIR) -type f -name *.java > $@
