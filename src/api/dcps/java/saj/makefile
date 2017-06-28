JAR_BASE       := dcpssaj

JCODE_COMMON_DIR =  ../common/java/code
JPACKAGES       = DDS org/opensplice/dds/dcps org/opensplice/dds/type

JAVA_DIRS       = $(shell find $(JCODE_DIR) -type d)
JAVA_DIRS      += $(shell find $(JCODE_COMMON_DIR) -type d)
JAVA_FILES_TMP  = $(wildcard $(addsuffix /*.java,$(addprefix $(JCODE_DIR)/,$(JPACKAGES))))
JAVA_FILES_TMP += $(wildcard $(addsuffix /*.java,$(addprefix $(JCODE_COMMON_DIR)/,$(JPACKAGES))))

IDL_FILES      := dds_builtinTopics.idl dds_namedQosTypes.idl
IDL_JAVA_FILES  = $(IDL_CODE_DIR)/idl_java_files.txt



JCODE_PATH      = $(JCODE_DIR):$(JCODE_COMMON_DIR):$(IDL_CODE_DIR)

JAR_DEPENDENCIES += $(IDL_TIMESTAMP)
SRC_JAR_DEPENDENCIES += $(IDL_TIMESTAMP)

all link: jar src

include ../common.mak

$(IDL_JAVA_FILES): $(IDL_FILES) | $(IDL_CODE_DIR) $(CLASS_DIR)
	@for f in $^ ; do \
		echo $(IDLPP) -l java -S -d $(IDL_CODE_DIR) `ospl_normalizePath $$f` ; \
		$(IDLPP) -l java -S -d $(IDL_CODE_DIR) `ospl_normalizePath $$f` ; \
	done
	find $(IDL_CODE_DIR) -type f -name *.java > $@
