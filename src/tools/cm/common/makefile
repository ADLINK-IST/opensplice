JAR_MODULE	 = tooling_common.jar
JAR_LOCATION = $(OSPL_HOME)
JAR_INC_DIR	 = $(OSPL_HOME)/jar/$(SPLICE_TARGET)

ifeq (,$(findstring win32,$(SPLICE_HOST))) 
JAVA_INC	=$(JAR_INC_DIR)/cmapi.jar
else
TMP_JAVA_INC	=$(JAR_INC_DIR)/cmapi.jar
JAVA_INC	=$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(TMP_JAVA_INC))
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
		
all link: jar

include $(OSPL_HOME)/setup/makefiles/target.mak


analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) 
