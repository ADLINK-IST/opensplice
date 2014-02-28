JAR_MODULE	 = tooling_common.jar
JAR_LOCATION = $(OSPL_HOME)
JAR_INC_DIR	 = $(OSPL_HOME)/jar/$(SPLICE_TARGET)

JAVA_INC	= $(JAR_INC_DIR)/cmapi.jar

TMP_CODE_DIR    = bld/$(SPLICE_TARGET)
JTIMESTAMP      = $(TMP_CODE_DIR)/.java_common.timestamp
ifndef OSPL_OUTER_HOME
EXTRA_SOURCE = $(OSPL_HOME)/src/tools/cm/common/etc/ConfigModeIntializer.java
else
EXTRA_SOURCE = $(OSPL_OUTER_HOME)/src/tools/cm/common/etc/ConfigModeIntializer.java
endif

JPACKAGES=	org/opensplice/common/util \
		org/opensplice/common \
		org/opensplice/common/model/table/status \
		org/opensplice/common/model/table/qos \
		org/opensplice/common/model/table \
		org/opensplice/common/model/sample \
		org/opensplice/common/model \
		org/opensplice/common/view \
		org/opensplice/common/view/table \
		org/opensplice/common/view/entity \
		org/opensplice/common/view/entity/tree \
		org/opensplice/common/controller

all link: $(JTIMESTAMP) jar

include $(OSPL_HOME)/setup/makefiles/target.mak


analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) manifest/$(SPLICE_TARGET)


$(JTIMESTAMP): $(EXTRA_SOURCE)
	@rm -rf $(TMP_CODE_DIR)
	@mkdir -p $(TMP_CODE_DIR)
	$(JCC) $(JCC_ARGS) $(EXTRA_SOURCE)
	@touch $(JTIMESTAMP)
