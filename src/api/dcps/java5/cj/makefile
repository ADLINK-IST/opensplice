.NOTPARALLEL:

JAR_BASE		= dcpscj5
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

JAVA_ORB = JACORB

JCODE_DIR	= ../common/java/code
CJ_HOME		=$(OSPL_HOME)/src/api/dcps/java/cj
CJ_JAR		=$(JAR_INC_DIR)/dcpscj.jar
CJ_PACKAGES	=bld/$(SPLICE_TARGET)/DDS
JAVA_INC	=bld/$(SPLICE_TARGET)

SRC_JAR     =$(JAR_INC_DIR)/$(SRC_JAR_MODULE)
TMP_SRC_JAR =manifest/$(SPLICE_TARGET)/$(SRC_JAR_MODULE)

TMP_DOC_DIR   =manifest/$(SPLICE_TARGET)/docs
DOC_DIR       =$(OSPL_HOME)/docs/java5
DOC_JAR       =$(DOC_DIR)/$(DOC_JAR_MODULE)
DOC_JPACKAGES = $(subst /,.,$(JPACKAGES))

COMMON_CODE_DIR	=../common/java/code
CJ5_CODE_DIR	=code
JCODE_PATH      = $(COMMON_CODE_DIR):$(CJ5_CODE_DIR)

JAVA_FILES_CJ5	= $(wildcard $(addsuffix /*.java,$(addprefix $(CJ5_CODE_DIR)/,$(JPACKAGES))))
CLASS_FILES_CJ5	= $(subst .java,.class,$(subst $(CJ5_CODE_DIR),$(CLASS_DIR),$(JAVA_FILES_CJ5)))

JAR_DEPENDENCIES=$(CJ_PACKAGES) $(CLASS_FILES_CJ5)

MANIFEST_TEMPLATE = meta-inf/manifest-osgi-bundle.tmpl

all link: jar $(SRC_JAR)

doc: $(DOC_JAR)

include $(OSPL_HOME)/setup/makefiles/target.mak

$(CLASS_FILES_CJ5): $(JAVA_FILES_CJ5) $(CLASS_DIR)/.STAMP
	$(JCC) $(JCC_ARGS) code/org/opensplice/dds/core/*.java

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
$(SRC_JAR): $(JAVA_FILES_CJ5) $(JAR_TARGET) $(CLASS_FILES)
	$(JAR) cf $(TMP_SRC_JAR) -C $(CJ5_CODE_DIR) .
	$(JAR) uf $(TMP_SRC_JAR) -C $(COMMON_CODE_DIR) .
	$(JAR) uf $(TMP_SRC_JAR) -C $(CJ_HOME)/bld/code/$(SPLICE_TARGET) .
	$(JAR) uf $(TMP_SRC_JAR) -C $(CJ_HOME)/code .
	$(JAR) uf $(TMP_SRC_JAR) -C $(CJ_HOME)/../common/java/code .
	mv $(TMP_SRC_JAR) $(JAR_INC_DIR)
	@touch $(JAR_INC_DIR)/$(SRC_JAR_MODULE)
else
$(SRC_JAR): $(JAVA_FILES_CJ5) $(JAR_TARGET) $(CLASS_FILES)
	$(JAR) cf $(shell cygpath -m $(TMP_SRC_JAR)) -C $(shell cygpath -m $(CJ5_CODE_DIR)) .
	$(JAR) uf $(shell cygpath -m $(TMP_SRC_JAR)) -C $(shell cygpath -m $(COMMON_CODE_DIR)) .
	$(JAR) uf $(shell cygpath -m $(TMP_SRC_JAR)) -C $(shell cygpath -m $(CJ_HOME)/bld/code/$(SPLICE_TARGET)) .
	$(JAR) uf $(shell cygpath -m $(TMP_SRC_JAR)) -C $(shell cygpath -m $(CJ_HOME)/code) .
	$(JAR) uf $(shell cygpath -m $(TMP_SRC_JAR)) -C $(shell cygpath -m $(CJ_HOME)/../common/java/code) .
	mv $(TMP_SRC_JAR) $(JAR_INC_DIR)
	@touch $(JAR_INC_DIR)/$(SRC_JAR_MODULE)
endif

$(DOC_DIR)/.STAMP:
	mkdir -p $(DOC_DIR)
	touch $@

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
$(DOC_JAR): $(JAVA_FILES_CJ5) $(JAR_TARGET) $(DOC_DIR)/.STAMP
	javadoc -sourcepath "$(JCODE_PATH)" -classpath "$(OSPL_HOME)/jar/$(SPLICE_TARGET)/dcpscj.jar:$(JACORB_HOME)/lib/endorsed/jacorb.jar" -quiet -d $(TMP_DOC_DIR) $(DOC_JPACKAGES)
	$(JAR) cf $(DOC_JAR) -C $(TMP_DOC_DIR) .
	@touch $(DOC_JAR)
else
$(DOC_JAR): $(JAVA_FILES_CJ5) $(JAR_TARGET) $(DOC_DIR)/.STAMP
	javadoc -sourcepath "$(shell cygpath -m $(COMMON_CODE_DIR));$(shell cygpath -m $(CJ5_CODE_DIR))" -classpath "$(shell cygpath -m $(OSPL_HOME)/jar/$(SPLICE_TARGET)/dcpscj.jar);$(shell cygpath -m $(JACORB_HOME)/lib/endorsed/jacorb.jar)" -quiet -d $(shell cygpath -m $(TMP_DOC_DIR)) $(DOC_JPACKAGES)
	$(JAR) cf $(shell cygpath -m $(DOC_JAR)) -C $(shell cygpath -m $(TMP_DOC_DIR)) .
	@touch $(DOC_JAR)
endif

$(CJ_JAR):
	@$(MAKE) -C $(CJ_HOME)

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
$(CJ_PACKAGES): $(CJ_JAR) $(CLASS_DIR)/.STAMP
	cd $(CLASS_DIR);jar xf $(CJ_JAR)
	@rm -rf $(CLASS_DIR)/META-INF
	@touch $(CJ_PACKAGES)
else
$(CJ_PACKAGES): $(CJ_JAR) $(CLASS_DIR)/.STAMP
	cd $(CLASS_DIR);jar xf $(shell cygpath -m $(CJ_JAR))
	@rm -rf $(CLASS_DIR)/META-INF
	@touch $(CJ_PACKAGES)
endif


clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) manifest/$(SPLICE_TARGET) $(DOC_JAR)
