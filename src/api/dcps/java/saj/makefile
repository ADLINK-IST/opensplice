JAR_MODULE      = dcpssaj.jar
JAR_LOCATION    = $(OSPL_HOME)

JCODE_DIR       = ../common/java/code
SAJ_CODE_DIR    = code
IDL_CODE_DIR    = bld/code/$(SPLICE_TARGET)
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

BUNDLE_FILE          = $(JAR_TARGET)/dcpssaj-osgi-bundle.jar
BUNDLE_MANIFEST      = manifest/$(SPLICE_TARGET)/manifest.mf.osgi
BUNDLE_MANIFEST_TMPL = meta-inf/manifest-osgi-bundle.tmpl

all link: idlpp $(JTIMESTAMP) jar jar-bundle

include $(OSPL_HOME)/setup/makefiles/target.mak

$(JTIMESTAMP): $(TOPICIDL) $(QOSIDL)
	@rm -rf $(IDL_CODE_DIR) $(CLASS_DIR)
	@mkdir -p $(IDL_CODE_DIR) $(CLASS_DIR)
	$(IDLPP) -l java -S -d $(IDL_CODE_DIR) `ospl_normalizePath $(TOPICIDL)`
	$(IDLPP) -l java -S -d $(IDL_CODE_DIR) `ospl_normalizePath $(QOSIDL)`
	$(JCC) $(JCC_ARGS) $(EXTRA_SOURCE)
	@touch $(JTIMESTAMP)

jar-bundle: $(BUNDLE_FILE)

$(BUNDLE_FILE): $(BUNDLE_MANIFEST)
ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
	$(JAR) cmf $(BUNDLE_MANIFEST) $(BUNDLE_FILE) -C bld/$(SPLICE_TARGET) .
else
	$(JAR) cmf $(BUNDLE_MANIFEST) $(shell cygpath -m $(BUNDLE_FILE)) -C bld/$(SPLICE_TARGET) .
endif

$(BUNDLE_MANIFEST):
	@sed "s@<ospl-version>@$(BASE_VERSION)@" < $(BUNDLE_MANIFEST_TMPL) > $(BUNDLE_MANIFEST)

idlpp:
	cd $(OSPL_HOME)/src/cpp; make
	cd $(OSPL_HOME)/src/tools/cppgen; make
	cd $(OSPL_HOME)/src/tools/idlpp; make

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) $(IDL_CODE_DIR) manifest/$(SPLICE_TARGET) $(BUNDLE_FILE) $(BUNDLE_MANIFEST)
