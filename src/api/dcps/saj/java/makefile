JAR_MODULE	 = dcpssaj.jar
JAR_LOCATION = `$(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_HOME)`
JPACKAGES	 = DDS org/opensplice/dds/dcps

all link: jar

include $(OSPL_HOME)/setup/makefiles/target.mak

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) 
