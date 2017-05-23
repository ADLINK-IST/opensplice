JAR_MODULE   =qosProvider.jar
JAR_INC_DIR  =$(OSPL_HOME)/jar/$(SPLICE_TARGET)
JAR_LOCATION =exec/

TEST_JAR_DIR=$(JAR_LOCATION)/jar/$(SPLICE_TARGET)

JAVA_INC	=$(JAR_INC_DIR)/dcpssaj.jar:bld/$(SPLICE_TARGET)

JAVA_MAIN_CLASS	=QosProviderJava
JPACKAGES	= .

all link: jar

include $(OSPL_HOME)/setup/makefiles/target.mak


clean:
	@rm -rf bld/$(SPLICE_TARGET) exec/
	@rm -rf manifest
run: jar
	cp $(OSPL_HOME)/jar/$(SPLICE_TARGET)/*.jar $(JAR_TARGET)
	$(JAVA) -jar $(JAR_TARGET)/$(JAR_MODULE)
