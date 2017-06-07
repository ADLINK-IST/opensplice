.NOTPARALLEL:

JAR_MODULE      = dcpssaj.jar
JAR_LOCATION    = $(OSPL_HOME)

JCODE_DIR       = ../common/java/code
SAJ_CODE_DIR    = code
IDL_CODE_DIR    = bld/code/$(SPLICE_TARGET)/DDS
JPACKAGES       = DDS org/opensplice/dds/dcps

# Input IDL files.
IDL_DIR		:= $(OSPL_HOME)/etc/idl
TOPICIDL	:= $(IDL_DIR)/dds_builtinTopics.idl
QOSIDL		:= $(IDL_DIR)/dds_namedQosTypes.idl

ifneq (,$(or $(findstring win32, $(SPLICE_TARGET)), $(findstring win64, $(SPLICE_TARGET))))
EXTRA_SOURCE    = $(IDL_CODE_DIR)/DDS/\*.java $(SAJ_CODE_DIR)/DDS/\*.java
else
EXTRA_SOURCE    = $(IDL_CODE_DIR)/DDS/*.java $(SAJ_CODE_DIR)/DDS/*.java
endif

JTIMESTAMP      = $(IDL_CODE_DIR)/.java_idl.timestamp
JCODE_PATH      = $(SAJ_CODE_DIR):$(IDL_CODE_DIR):$(JCODE_DIR)
IDLPP           = idlpp

MANIFEST_TEMPLATE = meta-inf/manifest-osgi-bundle.tmpl

all link: $(JTIMESTAMP) jar

include $(OSPL_HOME)/setup/makefiles/target.mak

$(JTIMESTAMP): $(TOPICIDL) $(QOSIDL)
	@rm -rf $(IDL_CODE_DIR) $(CLASS_DIR)
	@mkdir -p $(IDL_CODE_DIR) $(CLASS_DIR)
	$(IDLPP) -l java -S -d $(IDL_CODE_DIR) `ospl_normalizePath $(TOPICIDL)`
	$(IDLPP) -l java -S -d $(IDL_CODE_DIR) `ospl_normalizePath $(QOSIDL)`
	$(JCC) $(JCC_ARGS) $(EXTRA_SOURCE)
	@touch $(JTIMESTAMP)

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) $(IDL_CODE_DIR) $(MANIFEST)
