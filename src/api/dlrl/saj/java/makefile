JAR_MODULE	 =dlrlsaj.jar
JAR_LOCATION =$(OSPL_HOME)
JAR_INC_DIR	 =$(OSPL_HOME)/jar/$(SPLICE_TARGET)

JAVA_INC	= $(JAR_INC_DIR)/dcpssaj.jar

JPACKAGES	= DDS
JPACKAGES	+= DDS/IntMapRelationPackage
JPACKAGES	+= DDS/StrMapRelationPackage
JPACKAGES	+= org/opensplice/dds/dlrl
JPACKAGES	+= org/opensplice/dds/dlrl/metamodel

all link: jar

include $(OSPL_HOME)/setup/makefiles/target.mak

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) 
