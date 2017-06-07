.NOTPARALLEL:

JAR_BASE		= dcpssaj5
JAR_MODULE      = $(JAR_BASE).jar
SRC_JAR_MODULE  = $(JAR_BASE)-src.jar
DOC_JAR_MODULE  = $(JAR_BASE)-doc.jar
JAR_LOCATION    = $(OSPL_HOME)
JAR_INC_DIR		= $(OSPL_HOME)/jar/$(SPLICE_TARGET)
JPACKAGES    	= org/omg/dds/core \
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

JCODE_DIR	= ../common/java/code
SAJ_HOME	=$(OSPL_HOME)/src/api/dcps/java/saj
SAJ_JAR		=$(JAR_INC_DIR)/dcpssaj.jar
SAJ_PACKAGES    =bld/$(SPLICE_TARGET)/DDS
JAVA_INC	=bld/$(SPLICE_TARGET)

SRC_JAR     =$(JAR_INC_DIR)/$(SRC_JAR_MODULE)
TMP_SRC_JAR =manifest/$(SPLICE_TARGET)/$(SRC_JAR_MODULE)

TMP_DOC_DIR   =manifest/$(SPLICE_TARGET)/docs
DOC_DIR       =$(OSPL_HOME)/docs/java5
DOC_JAR       =$(DOC_DIR)/$(DOC_JAR_MODULE)
DOC_JPACKAGES = $(subst /,.,$(JPACKAGES))

COMMON_CODE_DIR	=../common/java/code
SAJ5_CODE_DIR	=code
JCODE_PATH      = $(COMMON_CODE_DIR):$(SAJ5_CODE_DIR)

JAVA_FILES_SAJ5		= $(wildcard $(addsuffix /*.java,$(addprefix $(SAJ5_CODE_DIR)/,$(JPACKAGES))))
CLASS_FILES_SAJ5	= $(subst .java,.class,$(subst $(SAJ5_CODE_DIR),$(CLASS_DIR),$(JAVA_FILES_SAJ5)))

JAR_DEPENDENCIES=$(SAJ_PACKAGES) $(CLASS_FILES_SAJ5)

MANIFEST_TEMPLATE = meta-inf/manifest-osgi-bundle.tmpl

all link: jar $(SRC_JAR)

doc: $(DOC_JAR)

include $(OSPL_HOME)/setup/makefiles/target.mak

$(CLASS_FILES_SAJ5): $(JAVA_FILES_SAJ5) $(CLASS_DIR)/.STAMP
	$(JCC) $(JCC_ARGS) code/org/opensplice/dds/core/*.java

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
$(SRC_JAR): $(JAVA_FILES_SAJ5) $(JAR_TARGET)/.STAMP $(CLASS_FILES)
	$(JAR) cf $(TMP_SRC_JAR) -C $(SAJ5_CODE_DIR) .
	$(JAR) uf $(TMP_SRC_JAR) -C $(COMMON_CODE_DIR) .
	$(JAR) uf $(TMP_SRC_JAR) -C $(SAJ_HOME)/bld/code/$(SPLICE_TARGET) .
	$(JAR) uf $(TMP_SRC_JAR) -C $(SAJ_HOME)/code .
	$(JAR) uf $(TMP_SRC_JAR) -C $(SAJ_HOME)/../common/java/code .
	mv $(TMP_SRC_JAR) $(JAR_INC_DIR)
	@touch $(JAR_INC_DIR)/$(SRC_JAR_MODULE)
else
$(SRC_JAR): $(JAVA_FILES_SAJ5) $(JAR_TARGET)/.STAMP $(CLASS_FILES)
	$(JAR) cf $(shell cygpath -m $(TMP_SRC_JAR)) -C $(shell cygpath -m $(SAJ5_CODE_DIR)) .
	$(JAR) uf $(shell cygpath -m $(TMP_SRC_JAR)) -C $(shell cygpath -m $(COMMON_CODE_DIR)) .
	$(JAR) uf $(shell cygpath -m $(TMP_SRC_JAR)) -C $(shell cygpath -m $(SAJ_HOME)/bld/code/$(SPLICE_TARGET)) .
	$(JAR) uf $(shell cygpath -m $(TMP_SRC_JAR)) -C $(shell cygpath -m $(SAJ_HOME)/code) .
	$(JAR) uf $(shell cygpath -m $(TMP_SRC_JAR)) -C $(shell cygpath -m $(SAJ_HOME)/../common/java/code) .
	mv $(TMP_SRC_JAR) $(JAR_INC_DIR)
	@touch $(JAR_INC_DIR)/$(SRC_JAR_MODULE)
endif

$(DOC_DIR)/.STAMP:
	mkdir -p $(DOC_DIR)
	touch $@

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
$(DOC_JAR): $(JAVA_FILES_SAJ5) $(JAR_TARGET) $(DOC_DIR)/.STAMP
	javadoc -sourcepath "$(JCODE_PATH)" -classpath "$(OSPL_HOME)/jar/$(SPLICE_TARGET)/dcpssaj.jar" -quiet -d $(TMP_DOC_DIR) $(DOC_JPACKAGES)
	$(JAR) cf $(DOC_JAR) -C $(TMP_DOC_DIR) .
	@touch $(DOC_JAR)
else
$(DOC_JAR): $(JAVA_FILES_SAJ5) $(JAR_TARGET) $(DOC_DIR)/.STAMP
	javadoc -sourcepath "$(shell cygpath -m $(COMMON_CODE_DIR));$(shell cygpath -m $(SAJ5_CODE_DIR))" -classpath "$(shell cygpath -m $(OSPL_HOME)/jar/$(SPLICE_TARGET)/dcpssaj.jar)" -quiet -d $(shell cygpath -m $(TMP_DOC_DIR)) $(DOC_JPACKAGES)
	$(JAR) cf $(shell cygpath -m $(DOC_JAR)) -C $(shell cygpath -m $(TMP_DOC_DIR)) .
	@touch $(DOC_JAR)
endif


$(SAJ_JAR):
	@$(MAKE) -C $(SAJ_HOME)

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
$(SAJ_PACKAGES): $(SAJ_JAR) $(CLASS_DIR)/.STAMP
	cd $(CLASS_DIR);jar xf $(SAJ_JAR)
	@rm -rf $(CLASS_DIR)/META-INF
	@touch $(SAJ_PACKAGES)
else
$(SAJ_PACKAGES): $(SAJ_JAR) $(CLASS_DIR)/.STAMP
	cd $(CLASS_DIR);jar xf $(shell cygpath -m $(SAJ_JAR))
	@rm -rf $(CLASS_DIR)/META-INF
	@touch $(SAJ_PACKAGES)
endif


clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) manifest/$(SPLICE_TARGET) $(DOC_JAR)
