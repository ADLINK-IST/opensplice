.NOTPARALLEL:
JAR_LOCATION 	:= $(OSPL_HOME)

JAR_BASE        := ddsface
JAR_MODULE      := $(JAR_BASE).jar
JAVA_INC        = $(JAR_TARGET)/dcpssaj5.jar

JPACKAGES       := org/vortex/FACE
JCODE_PATH      = $(JCODE_DIR)$(JSEPARATOR)$(GEN_JCODE_DIR)

# Input IDL files.
IDL_DIR            := $(OSPL_HOME)/etc/idl
FACE_COMMON_IDL    := $(IDL_DIR)/FACE_common.idl
FACE_TS_COMMON_IDL := $(IDL_DIR)/FACE_TS_common.idl

JTIMESTAMP      := bld/code/$(SPLICE_TARGET)/.java_idl.timestamp
CTIMESTAMP      := bld/$(SPLICE_TARGET)/.class_idl.timestamp
IDLPP           := idlpp

DOC_JAR_TARGET  := $(OSPL_HOME)/docs/face/java
DOC_JAR_FILE    := $(DOC_JAR_TARGET)/$(JAR_BASE)-doc.jar
DOC_TMP_DIR     := manifest/$(SPLICE_TARGET)/docs
DOC_JPACKAGES   := $(subst /,.,$(JPACKAGES))

GEN_JCODE_DIR   := bld/code/$(SPLICE_TARGET)
GEN_JPACKAGES   := FACE
GEN_JAVA_FILES  = $(wildcard $(addsuffix /*.java,$(addprefix $(GEN_JCODE_DIR)/,$(GEN_JPACKAGES))))

.PHONY: all link doc
all link: jar

doc: $(DOC_JAR_FILE)

JAR_DEPENDENCIES = $(CTIMESTAMP)

include $(OSPL_HOME)/setup/makefiles/target.mak

$(JTIMESTAMP): $(FACE_COMMON_IDL) $(FACE_TS_COMMON_IDL)
	@mkdir -p bld/code/$(SPLICE_TARGET)
	$(IDLPP) -S -l java -d $(GEN_JCODE_DIR) `ospl_normalizePath $(FACE_COMMON_IDL)`
	$(IDLPP) -S -l java -d $(GEN_JCODE_DIR) `ospl_normalizePath $(FACE_TS_COMMON_IDL)`
	@touch $@

$(CTIMESTAMP): $(CLASS_DIR)/.STAMP $(JTIMESTAMP)
	$(AT_SIGN)$(JCC) $(JCC_ARGS) $(GEN_JAVA_FILES)
	$(AT_SIGN)cp resources/vortex_face.xsd $(CLASS_DIR)
	@touch $@

$(DOC_JAR_TARGET)/.STAMP:
	@mkdir -p $(dir $@)
	@touch $@

$(DOC_JAR_FILE): $(JTIMESTAMP) $(DOC_JAR_TARGET)/.STAMP $(JAVA_FILES)
ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
	javadoc -sourcepath '$(JCODE_PATH)' -classpath '$(JAVA_INC)' -d $(DOC_TMP_DIR) $(DOC_JPACKAGES)
	$(JAR) cf $@ -C $(DOC_TMP_DIR) .
else
	javadoc -sourcepath '$(JCODE_PATH)' -classpath '$(shell cygpath -m $(JAVA_INC))' -d $(DOC_TMP_DIR) $(DOC_JPACKAGES)
	$(JAR) cf $(shell cygpath -m $@) -C $(DOC_TMP_DIR) .
endif

.PHONY: clean
clean:
	$(AT_SIGN)rm -rf bld/$(SPLICE_TARGET) $(GEN_JCODE_DIR) $(JAR_TARGET)/$(JAR_MODULE) $(IDL_CODE_DIR) $(MANIFEST) $(DOC_TMP_DIR) $(DOC_JAR_FILE)
