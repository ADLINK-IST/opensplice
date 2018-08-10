.NOTPARALLEL:

JAR_MODULE	 =osplconf.jar
JAR_LOCATION = $(OSPL_HOME)
JAR_INC_DIR	 =$(OSPL_HOME)/jar/$(SPLICE_TARGET)
JAVA_MAIN_CLASS  =org.opensplice.config.SpliceConfig

JAVA_INC = $(JAR_INC_DIR)/tooling_common.jar

CONFIG_DIR       =$(OSPL_HOME)/src/tools/cm/config
META_SRC_DIR     =$(CONFIG_DIR)/code
META_BLD_DIR     =$(CONFIG_DIR)/bld/$(SPLICE_TARGET)
META_FILE        =splice_metaconfig_6.1.xml
META_XSD_FILE    =splice_metaconfig_6.1.xsd
DST_META_FILE    =ospl_metaconfig.xml
DST_META_XSD_FILE=ospl_metaconfig.xsd
META_FILES       =$(META_BLD_DIR)/$(DST_META_FILE)
META_FILES      +=$(META_BLD_DIR)/$(DST_META_XSD_FILE)

JPACKAGES=org/opensplice/config/meta \
		  org/opensplice/config/data \
		  org/opensplice/config/swing \
		  org/opensplice/config

LOGOS = bld/$(SPLICE_TARGET)/resources/
LOGO_SRC = resources/*.png

all link: $(META_FILES) $(LOGOS) jar

include $(OSPL_HOME)/setup/makefiles/target.mak

$(LOGOS): $(LOGO_SRC)
	@mkdir -p bld/$(SPLICE_TARGET)/resources
	@cp $(LOGO_SRC) $(LOGOS)

$(META_FILES): $(CLASS_DIR)/.STAMP $(META_SRC_DIR)/$(META_FILE) $(META_SRC_DIR)/$(META_XSD_FILE)
	cp $(META_SRC_DIR)/$(META_FILE) $(META_BLD_DIR)/$(DST_META_FILE)
	chmod +w $(META_BLD_DIR)/$(DST_META_FILE)
	cp $(META_SRC_DIR)/$(META_XSD_FILE) $(META_BLD_DIR)/$(DST_META_XSD_FILE)
	chmod +w $(META_BLD_DIR)/$(DST_META_XSD_FILE)
	@rm -rf $(CONFIG_DIR)/manifest/$(SPLICE_TARGET)
	@rm -f $(JAR_INC_DIR)/$(JAR_MODULE)


clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) manifest/$(SPLICE_TARGET)

run: jar
	$(JAVA) -jar $(JAR_TARGET)/$(JAR_MODULE)

