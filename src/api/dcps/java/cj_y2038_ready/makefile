.NOTPARALLEL:

JAR_MODULE      = dcpscj_y2038_ready.jar
JAR_LOCATION    = $(OSPL_HOME)

JCODE_DIR       = ../common/java/code
CJ_CODE_DIR     = ../cj/code
IDL_CODE_DIR    = bld/code/$(SPLICE_TARGET)/DDS
JPACKAGES       = DDS org/opensplice/dds/dcps

# Input IDL files.
IDL_DIR		:= $(OSPL_HOME)/etc/idl
TOPICIDL	:= $(IDL_DIR)/dds_builtinTopics.idl
QOSIDL		:= $(IDL_DIR)/dds_namedQosTypes.idl

TOPIC_DCPSIDL	= $(IDL_CODE_DIR)/$(subst .idl,Dcps.idl,$(notdir $(TOPICIDL)))
QOS_DCPSIDL		= $(IDL_CODE_DIR)/$(subst .idl,Dcps.idl,$(notdir $(QOSIDL)))
ifneq (,$(or $(findstring win32, $(SPLICE_TARGET)), $(findstring win64, $(SPLICE_TARGET))))
EXTRA_SOURCE    = $(IDL_CODE_DIR)/DDS/\*.java
else
EXTRA_SOURCE    = $(IDL_CODE_DIR)/DDS/*.java
endif

# JACORB
JAVA_ORB = JACORB

JTIMESTAMP      = $(IDL_CODE_DIR)/.java_idl.timestamp
JCODE_PATH      = $(CJ_CODE_DIR):$(IDL_CODE_DIR):$(JCODE_DIR)
IDLPP           = idlpp

MANIFEST_TEMPLATE = meta-inf/manifest-osgi-bundle.tmpl

all link: $(JTIMESTAMP) jar

include $(OSPL_HOME)/setup/makefiles/target.mak

$(JTIMESTAMP): $(TOPICIDL)
	@rm -rf $(IDL_CODE_DIR) $(CLASS_DIR)
	@mkdir -p $(IDL_CODE_DIR) $(CLASS_DIR)
	$(IDLPP) -l java -C -d $(IDL_CODE_DIR) `ospl_normalizePath $(TOPICIDL)`
	$(IDLPP) -l java -C -d $(IDL_CODE_DIR) `ospl_normalizePath $(QOSIDL)`
	$(JAVA) $(CP_JACORB_IDL) org.jacorb.idl.parser -DOSPL_DDS_Y2038_READY -I"$(OSPL_HOME_NORMALIZED)/etc/idl" -all -d $(IDL_CODE_DIR) $(TOPIC_DCPSIDL)
	$(JAVA) $(CP_JACORB_IDL) org.jacorb.idl.parser -I"$(OSPL_HOME_NORMALIZED)/etc/idl" -all -d $(IDL_CODE_DIR) $(QOS_DCPSIDL)
	$(JCC) $(JCC_ARGS) $(EXTRA_SOURCE)
	@touch $(JTIMESTAMP)

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) $(IDL_CODE_DIR)

