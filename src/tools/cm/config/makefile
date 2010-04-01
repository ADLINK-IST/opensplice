JAR_MODULE	 =osplconf.jar
JAR_LOCATION = $(OSPL_HOME)
JAR_INC_DIR	 =$(OSPL_HOME)/jar/$(SPLICE_TARGET)
JAVA_MAIN_CLASS  =org.opensplice.config.SpliceConfig
ifeq (,$(findstring win32,$(SPLICE_HOST)))
   JAVA_INC	=$(JAR_INC_DIR)/tooling_common.jar
else
   JAVA_INC	=$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(JAR_INC_DIR)/tooling_common.jar)
endif
CONFIG_DIR       =$(OSPL_HOME)/src/tools/cm/config
META_SRC_DIR     =$(CONFIG_DIR)/code
META_BLD_DIR     =$(CONFIG_DIR)/bld/$(SPLICE_TARGET)
META_FILE        =splice_metaconfig_5.1.xml
META_XSD_FILE    =splice_metaconfig_5.1.xsd
META_FILES       =$(META_BLD_DIR)/$(META_FILE)
META_FILES      +=$(META_BLD_DIR)/$(META_XSD_FILE)

JPACKAGES=org/opensplice/config/meta \
		  org/opensplice/config/data \
		  org/opensplice/config/swing \
		  org/opensplice/config

JFLAGS=-source 1.5 -target 1.5

all link: $(META_FILES) jar

include $(OSPL_HOME)/setup/makefiles/target.mak

$(META_FILES): $(CLASS_DIR) $(META_SRC_DIR)/$(META_FILE) $(META_SRC_DIR)/$(META_XSD_FILE)
	cp $(META_SRC_DIR)/$(META_FILE) $(META_BLD_DIR)/$(META_FILE)
	chmod +w $(META_BLD_DIR)/$(META_FILE)
	cp $(META_SRC_DIR)/$(META_XSD_FILE) $(META_BLD_DIR)/$(META_XSD_FILE)
	chmod +w $(META_BLD_DIR)/$(META_XSD_FILE)
	@rm -rf $(CONFIG_DIR)/manifest/$(SPLICE_TARGET)
	@rm -f $(JAR_INC_DIR)/$(JAR_MODULE)

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) manifest/$(SPLICE_TARGET)

run: jar
	$(JAVA) -jar $(JAR_TARGET)/$(JAR_MODULE)

